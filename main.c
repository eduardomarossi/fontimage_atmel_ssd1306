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

int main(int argc, char* argv[]) {
    TTF_Font      *font;
    SDL_Surface   *font_surface;
    SDL_Color     fg = {255, 255, 255};
    SDL_Color     bg = {0, 0, 0};

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

    //test_render(10, 16, fontd); // Habilite para ver como a biblioteca da Atmel renderiza o caractere.

    // Render
    font_surface = TTF_RenderText_Shaded(font, argv[3], fg, bg);

    // Write BMP
    SDL_LockSurface(font_surface);
    // write c and h files

    printf("font size: %d x %d\n", font_surface->w, font_surface->h);
    uint8_t  data      = 0x00;
    uint32_t threshold = 10;
    int pixelsRow = 0;
    int prints = 0;

    for(int i = 0; i < font_surface->h; i++) {

        for(int j = 0; j < font_surface->w; j++) {
            if(pixelsRow == 8) {
                printf("0x%x, ", data);
                data = 0x00;
                pixelsRow = 0;
                prints++;
            }

            if(getpixel(font_surface, j, i) > threshold) {
                data |= (0x01) << (7 - (j % 8));

            }

            pixelsRow++;
        }

        if(pixelsRow > 0) {
            printf("0x%x, ", data);
            data = 0x00;
            pixelsRow = 0;
            prints++;
        }

    }

    printf("\nsize: %d bytes", prints);
    // Renderizar caractere por caractere


    SDL_SaveBMP(font_surface, "teste.bmp");
    SDL_UnlockSurface(font_surface);
    SDL_Quit();
}