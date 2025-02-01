#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// Definições
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define tempo 2000

const uint ledRed_pin = 13;
const uint button_A = 5;
const uint button_B = 6;

static volatile int cont = 1;
static volatile uint32_t last_time_A = 0;
static volatile uint32_t last_time_B = 0;

uint8_t led_r = 0; 
uint8_t led_g = 0; 
uint8_t led_b = 100; 

// Envia uma cor para o painel de LEDs
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Converte valores de RGB para 32 bits, no formato usado pelos LEDs WS2812.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Paineis
uint8_t desenho0[25] = { 0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0, 
                        0, 1, 0, 1, 0,
                        0, 1, 0, 1, 0,
                        0, 1, 1, 1, 0 };

uint8_t desenho1[25] =   {0, 1, 1, 1, 0,
                        0, 0, 1, 0, 0, 
                        0, 0, 1, 0, 0,
                        0, 1, 1, 0, 0,
                        0, 0, 1, 0, 0};

uint8_t desenho2[25] =   {0, 1, 1, 1, 0,
                        0, 1, 0, 0, 0, 
                        0, 0, 1, 0, 0,
                        0, 0, 0, 1, 0,
                        0, 1, 1, 1, 0};
                                         
uint8_t desenho3[25] =   {0, 1, 1, 1, 0,
                        0, 0, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 0, 0, 1, 0,
                        0, 1, 1, 1, 0};

uint8_t desenho4[25] =   {0, 1, 0, 0, 0,
                        0, 0, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0,
                        0, 1, 0, 1, 0};

uint8_t desenho5[25] =  {0, 1, 1, 1, 0,
                        0, 0, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 1, 0, 0, 0,
                        0, 1, 1, 1, 0};


uint8_t desenho6[25] =   {0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 1, 0, 0, 0,
                        0, 1, 1, 1, 0};

uint8_t desenho7[25] =   {0, 1, 0, 0, 0,
                        0, 0, 0, 1, 0, 
                        0, 1, 0, 0, 0,
                        0, 1, 0, 1, 0,
                        0, 1, 1, 1, 0};
                                         
uint8_t desenho8[25] =   {0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0,
                        0, 1, 1, 1, 0};

uint8_t desenho9[25] =   {0, 1, 0, 0, 0,
                        0, 0, 0, 1, 0, 
                        0, 1, 1, 1, 0,
                        0, 1, 0, 1, 0,
                        0, 1, 1, 1, 0};

uint8_t *desenhos[] = {desenho0, desenho1, desenho2, desenho3, desenho4, desenho5, desenho6, desenho7, desenho8, desenho9};

// Função pata exibir os desenhos no painel de LED
void display_number(int num, uint8_t r, uint8_t g, uint8_t b) {
    if (num < 0 || num > 9) return; 

    uint32_t color = urgb_u32(r, g, b);
    
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (desenhos[num][i]) {
            put_pixel(color);
        } else {
            put_pixel(0);
        }
    }
}

// Função de interrupção para quando os botões forem pressionados, aumentam o decrescem o contador que define qual painel aparecerá
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    // Verificação e debouncing de 300ms
    if (gpio == button_A && (current_time - last_time_A > 300000)) {
        last_time_A = current_time;
        if (cont > 0) cont--; 
        printf("Botão A pressionado! Contador = %d\n", cont);
    }
    else if 
        (gpio == button_B && (current_time - last_time_B > 300000)) {
        last_time_B = current_time;
        if (cont < 9) cont++; 
        printf("Botão B pressionado! Contador = %d\n", cont);
    }
}

int main() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    stdio_init_all();

    gpio_init(ledRed_pin);
    gpio_init(button_A);
    gpio_init(button_B);

    gpio_set_dir(ledRed_pin, GPIO_OUT);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);

    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    int last_cont = -1;

    while (true) {
        static bool led_on = false;
        gpio_put(ledRed_pin, led_on);
        led_on = !led_on;
        sleep_ms(100);

        if (cont != last_cont) {  
            printf("Atualizando painel: %d\n", cont);
            display_number(cont, led_r, led_g, led_b);
            last_cont = cont;
        }
    }
}
