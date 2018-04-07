#include <stdio.h>
#include <SDL_video.h>
#include <SDL.h>
#include <SDL_ttf.h>

// Metódo para testar se está no formato da biblioteca do Atmel SSD1306 (desenha na tela)
// Basta indicar tamanho W e H do caractere e vetor do caractere

void test_render(int font_w, int font_h, uint8_t font[]) {

    SDL_Window    *window;
    SDL_Renderer  *renderer;

    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, 0);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


    int y = 0;
    int x = 0;

    // offset tela
    int ow = 300;
    int oh = 300;
    uint8_t *p = font;

    for(int i = 0; i < font_h; i++) {
        uint8_t data;

        x = 0;
        for(int j = 0; j < font_w; j++) {
            if(j % 8 == 0) {
                data = *p;
                p++;
            }

            if(data & 0x80) {
                SDL_RenderDrawPoint(renderer, ow + x, oh + y);
            }
            x++;
            data <<= 1;
        }
        y += 1;
        x = 0;
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
}


Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    Uint8 *pixels;
    switch(bpp) {
        case 1:
            //Convert the pixels to 32 bit
            pixels = (Uint8 *)surface->pixels;

            //Get the requested pixel
            //return pixels[ ( y * surface->w ) + x ];

            //printf("oi1");
            return *p;
            //break;

        case 2:
            printf("oi2");
            return *(Uint16 *)p;
            break;

        case 3:
            printf("oi3");
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
            break;

        case 4:
            printf("oi4");
            return *(Uint32 *)p;
            break;

        default:
            return 0;       /* shouldn't happen, but avoids warnings */
    }
}


static int BYTE_PRINTS = 0;
void print_byte(uint8_t data) {
    printf("0x%02X, ", data);
    BYTE_PRINTS++;
    if(BYTE_PRINTS > 8) {
        BYTE_PRINTS = 0;
        printf("\\ \n");
    }
}

void char_size(TTF_Font* font, int *w, int *h) {
    SDL_Surface   *font_surface;
    SDL_Color bg = {0, 0, 0};
    SDL_Color fg = {255, 255, 255};
    font_surface = TTF_RenderText_Shaded(font, "0", fg, bg);
    *w = font_surface->w;
    *h = font_surface->h;
}

void print_char(char symbol, TTF_Font* font, uint8_t threshold, SDL_Color* fg, SDL_Color* bg) {
    SDL_Surface   *font_surface;

    char str[] = {0, '\0'};

    uint8_t  data      = 0x00;
    int pixelsRow      = 0;
    int prints         = 0;

    str[0] = symbol;
    font_surface = TTF_RenderText_Shaded(font, str, *fg, *bg);
    SDL_LockSurface(font_surface);

    for(int i = 0; i < font_surface->h; i++) {

        for(int j = 0; j < font_surface->w; j++) {
            if(pixelsRow == 8) {
                print_byte(data);
                data = 0x00;
                pixelsRow = 0;
            }

            if(getpixel(font_surface, j, i) > threshold) {
                data |= (0x01) << (7 - (j % 8));

            }

            pixelsRow++;
        }

        if(pixelsRow > 0) {
            print_byte(data);
            data = 0x00;
            pixelsRow = 0;
        }

    }

    SDL_UnlockSurface(font_surface);
    SDL_FreeSurface(font_surface);
}

void print_char_array(char characters[], TTF_Font *font, uint8_t threshold, SDL_Color* fg, SDL_Color* bg) {
    int len = strlen(characters);
    int font_w, font_h;

    if(!len) return;

    char_size(font, &font_w, &font_h);
    printf("#define  SYSFONT_WIDTH   %d \n", font_w);
    printf("#define  SYSFONT_HEIGHT  %d \n", font_h);
    printf("#define  LINE_SPACING    %d \n", 8);
    printf("#define SYSFONT_FIRSTCHAR       ((uint8_t)'%c') \n", characters[0]);
    printf("#define SYSFONT_LASTCHAR       ((uint8_t)'%c') \n", characters[len-1]);
    printf("#define SYSFONT_DEFINE_GLYPHS \\ \n");
    printf("static PROGMEM_DECLARE(uint8_t, sysfont_glyphs[]) = { \\ \n");
    for(int i = 0; i < len; i++) {
        print_char(characters[i], font, threshold, fg, bg);
        printf("/* %c */ \\ \n", characters[i]);
        BYTE_PRINTS = 0;
    }
    printf("};");
}

int main(int argc, char* argv[]) {
    SDL_Color     fg = {255, 255, 255};
    SDL_Color     bg = {0, 0, 0};
    TTF_Font*     font;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("Erro ao inicializar SDL");
        return 1;
    }

    if(TTF_Init()) {
        printf("Erro ao inicializar SDL TTF");
        return 1;
    }

    if(argc < 4) {
        printf("Faltando argumentos.\n");
        printf("fontimage_atmel_ssd1306 Fonte.ttf Tamanho Caracteres\n");
        printf("Exemplo: fontimage_atmel_ssd1306 BPmono.ttf 32 ABCDEF\n");
        return 1;
    }

    int fontsize = strtol(argv[2], NULL, 10);
    font = TTF_OpenFont(argv[1], fontsize);
    if(!font) {
        printf("Erro ao carregar fonte");
        return 1;
    }

    //print_char('5', font, 10, &fg, &bg);
    print_char_array(argv[3], font, 10, &fg, &bg);
    SDL_Quit();
}