#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <queue.h>

static struct can2040 cbus;

QueueHandle_t messages;


// When a message is recieved by CAN bus, send it into our messages queue to be handled
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    xQueueSendToBack(messages, msg, portMAX_DELAY);
}

void receive_loop(__unused void *params)
{
    while (1)
    {
        struct can2040_msg msg;
        xQueueReceive(messages, &msg, portMAX_DELAY);
        printf("Received message: ");
        printf("%d%d%d%d\n", 
        (msg.data[0]), 
        (msg.data[1]), 
        (msg.data[2]), 
        (msg.data[3]));
    }
}

void transmit_loop(__unused void *params)
{
    struct can2040_msg message;

    // Create the queue to hold max of 50 CAN messages
    message.id = 0;
    message.dlc = 8;
    message.data[0] = 5;
    message.data[1] = 6;
    message.data[2] = 7;
    message.data[3] = 8;
    
    while (1)
    {
        can2040_transmit(&cbus, &message);
        vTaskDelay(1);
    }
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
    irq_set_enabled(PIO0_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

void main (void) {
    stdio_init_all();

    messages = xQueueCreate(50, sizeof(struct can2040_msg));

    canbus_setup();
    TaskHandle_t task_receiver;
    TaskHandle_t task_transmitt;

    xTaskCreate(receive_loop, "ReceiverThread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &task_receiver);
    xTaskCreate(transmit_loop, "TransmitterThread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &task_transmitt);

    vTaskStartScheduler();

    return 0;
}



