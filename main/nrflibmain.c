// /* Include library */
// #include "include/nrf24l01p_lib.h"
// #include <stdio.h>
// #include <driver/spi_master.h>

// /* Include FreeRTOS for delays */
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// /* Default SPI pins for HSPI */
// #define SPI_SCLK 14
// #define SPI_MISO 12
// #define SPI_MOSI 13

// /* How to determine sender and receiver */
// #define NRF_MODE_SENDER 1
// #define NRF_MODE_RECEIVER 2

// /* Set your mode here */
// #define NRF_MODE NRF_MODE_SENDER

// void app_main(void)
// {

//     printf("app-maine girildi");
//     /* Init spi bus */
//     spi_bus_config_t buscfg;
//     memset(&buscfg, 0, sizeof(buscfg));

//     buscfg.miso_io_num = SPI_MISO;
//     buscfg.mosi_io_num = SPI_MOSI;
//     buscfg.sclk_io_num = SPI_SCLK;
//     buscfg.quadhd_io_num = -1;
//     buscfg.quadwp_io_num = -1;
//     buscfg.max_transfer_sz = 4096;

//     esp_err_t err = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
//     assert(err == ESP_OK);

//     /* Create instance of CNRFLib */
//     CNRFLib nrf;
//     CNRFLib_Init(&nrf, GPIO_NUM_16, GPIO_NUM_17);
//     /* Attach to spi bus */
//     CNRFLib_AttachToSpiBus(&nrf, SPI3_HOST);

//     /* Buffer for tx/rx operations */
//     uint8_t buff[32];
//     /* Setup custom address */
//     uint8_t addr[5] = {222, 111, 001, 100, 040};

//     if (NRF_MODE == NRF_MODE_SENDER)
//     {
//         printf("Tx mode\n");

//         /* Fill buffer with dummy content */
//         for (int i = 0; i < 32; i++)
//             buff[i] = i;

//         /* Init library with Begin() */
//         CNRFLib_Begin(&nrf, nrf_tx_mode);
//         printf("NRF initiated");
//         /* Set addr to which we will send data */
//         SetTxAddr(&nrf, addr, 5);
//         /* Set pip0 addr to the same value to receive acks */
//         SetPipeAddr(&nrf, 0, addr, 5);

//         int8_t result;
//         while (1)
//         {
//             /* Check out Send function description */
//             result = CNRFLib_Send(&nrf, buff, 32);
//             buff[1]++;

//             printf("Result: %d\n", result);
//             vTaskDelay(pdMS_TO_TICKS(500));
//         }
//     }
//     else
//     {
//         printf("Rx mode\n");

//         /* Init library */
//         CNRFLib_Begin(&nrf, nrf_rx_mode);
//         /* Set pipe0 addr to listen to packets with this addr */
//         SetPipeAddr(&nrf, 0, addr, 5);

//         while (1)
//         {
//             vTaskDelay(pdMS_TO_TICKS(1));

//             /* Check for available packets in rx buffer */
//             if (IsRxDataAvailable(&nrf))
//                 continue;

//             /* Read it */
//             CNRFLib_Read(&nrf, buff, 32);
//             printf("Received: ");
//             for (int i = 0; i < 32; i++)
//                 printf("%d ", buff[i]);
//             printf("\n");
//         }
//     }
// }