/*
Copyright 2018 Shitov Egor

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "include/nrf24l01p_lib.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define DELAY_MS(x) vTaskDelay(pdMS_TO_TICKS((x)))
#define _TOUINT(x) (*((uint8_t *)(&(x))))

void CNRFLib_Init(CNRFLib *lib, gpio_num_t cs, gpio_num_t ce)
{
    lib->m_CS = cs;
    lib->m_CE = ce;
    lib->isAttached = false;
    lib->m_Mode = nrf_mode_none;
}
esp_err_t CNRFLib_AttachToSpiBus(CNRFLib *lib, spi_host_device_t spiBus)
{
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));

    devcfg.clock_speed_hz = 1 * 1000 * 1000; // 1 MHz
    devcfg.mode = 0;                         // SPI mode 0
    devcfg.spics_io_num = -1;                // CS pin handled manually
    devcfg.queue_size = 8;                   // Transaction queue size
    devcfg.duty_cycle_pos = 128;             // Duty cycle 50%

    esp_err_t err = spi_bus_add_device(spiBus, &devcfg, &lib->m_SpiDevice);
    if (err == ESP_OK)
        lib->isAttached = true;

    return err;
}

void CNRFLib_Begin(CNRFLib *lib, nrf_mode_t mode)
{
    assert(lib->isAttached && "Call AttachToSpiBus first");
    assert(mode == nrf_tx_mode || mode == nrf_rx_mode);

    CELow(lib);
    CSHigh(lib);

    DELAY_MS(5);

    /* Enable each pipes for receiving */
    WriteReg(lib,NRF_REG_ENRXADDR, 0x3F);

    /* Enabled auto acknowledgement */
    WriteReg(lib,NRF_REG_ENAA, 0x3F);

    /* Set payload width to each pipe to 32 bytes */
    for (int i = 0; i < 6; i++)
        WriteReg(lib,NRF_REG_RX_PW_P0, NRF_MAX_PAYLOAD);

    CNRFLib_FlushTX(lib);
    CNRFLib_FlushRX(lib);

    WriteReg(lib,NRF_REG_SETUP_ADDR_W, 0b11);
    SetAutoRetransmission(lib,10, 5);
    SetChannel(lib,115);
    SetDataRateAndPower(lib,NRF_DATA_RATE_250k, NRF_POWER_0dBm);

    /* Setup config register */
    nrf_reg_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.en_crc = NRF_ENABLE_CRC;
    cfg.crco = NRF_CRC_WIDTH;
    cfg.pwr_up = mode == nrf_rx_mode;
    cfg.prim_rx = mode == nrf_rx_mode;
    WriteReg(lib,NRF_REG_CONFIG, _TOUINT(cfg));

    if (mode == nrf_rx_mode)
        CEHigh(lib);

    lib->m_Mode = mode;

    /* Start up delay */
    DELAY_MS(2);
#ifdef DEBUG_MODE
    PrintRegister(GetStatus());
    PrintRegister(GetFifoStatus());

    memset(&cfg, 0, sizeof(cfg));
    uint8_t tmp = ReadReg(NRF_REG_CONFIG);
    memcpy(&cfg, &tmp, sizeof(uint8_t));
    printf("en_crc: %d crc: %d pwr_up: %d prim_rx: %d\n", cfg.en_crc, cfg.crco, cfg.pwr_up, cfg.prim_rx);

    uint8_t rxAddr[5] = {0};
    uint8_t txAddr[5] = {0};

    ReadBytes(NRF_CMD_R_REGISTER | NRF_REG_RX_ADDR_P0, rxAddr, 5);
    ReadBytes(NRF_CMD_R_REGISTER | NRF_REG_TX_ADDR, txAddr, 5);

    printf("rx addr pipe 0: ");
    for (int i = 0; i < 5; i++)
        printf("%d", rxAddr[i]);

    printf("\n");
    printf("tx addr: ");
    for (int i = 0; i < 5; i++)
        printf("%d", txAddr[i]);
    printf("\n");

    nrf_reg_en_rxaddr_t en;
    memset(&en, 0, sizeof(en));
    tmp = ReadReg(NRF_REG_ENRXADDR);
    memcpy(&en, &tmp, sizeof(uint8_t));
    printf("pipe 0 enabled: %d\n", en.enrxaddr_pipe0);
#endif /* DEBUG_MODE */
}

uint8_t ReadReg(CNRFLib *lib,uint8_t reg)
{
    uint8_t result;
    ReadBytes(lib,NRF_CMD_R_REGISTER | reg, &result, 1);
    return result;
}

uint8_t WriteReg(CNRFLib *lib,uint8_t reg, uint8_t value)
{
    return WriteBytes(lib,NRF_CMD_W_REGISTER | reg, &value, 1);
}

uint8_t ReadBytes(CNRFLib *lib,uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow(lib);
    esp_rom_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd, lib->m_SpiDevice);
    while (pBuff && length--)
    {
        esp_rom_delay_us(1);
        *pBuff++ = spi_transfer_byte(0xFF, lib->m_SpiDevice);
    }
    esp_rom_delay_us(3);
    CSHigh(lib);

    return status;
}

uint8_t WriteBytes(CNRFLib *lib,uint8_t cmd, uint8_t *pBuff, uint8_t length)
{
    CSLow(lib);
    esp_rom_delay_us(3);
    uint8_t status = spi_transfer_byte(cmd, lib->m_SpiDevice);
    while (pBuff && length--)
    {
        esp_rom_delay_us(1);
        spi_transfer_byte(*pBuff++, lib->m_SpiDevice);
    }
    esp_rom_delay_us(3);
    CSHigh(lib);

    return status;
}

int8_t CNRFLib_Send(CNRFLib *lib, uint8_t *pBuff, uint8_t length)
{
    assert(lib->isAttached && "Call AttachToSpiBus first");
    assert(lib->m_Mode == nrf_tx_mode && "Tx mode must be set");

    CELow(lib);
    CNRFLib_FlushTX(lib);

    /* Setup as tx if needed and wakeup */
    uint8_t tmp = ReadReg(lib,NRF_REG_CONFIG);
    nrf_reg_config_t cfg = *(nrf_reg_config_t *)&tmp;
    cfg.pwr_up = 1;  /* Wakeup */
    cfg.prim_rx = 0; /* Set tx mode if needed */
    WriteReg(lib,NRF_REG_CONFIG, _TOUINT(cfg));
    DELAY_MS(2); // wakeup delay

    WriteTxPayload(lib,pBuff, length);

    /* Set CE high for 10 us atleast */
    CEHigh(lib);
    esp_rom_delay_us(30);
    CELow(lib);

    /* Wait for success or fail */
    nrf_reg_status_t status;
    do
    {
        status = CNRFLib_GetStatus(lib);
        esp_rom_delay_us(20);
    } while (status.max_rt == 0 && status.tx_ds == 0);

    /* Read retries count */
    nrf_reg_observe_tx_t observe;
    memset(&observe, 0, sizeof(observe));
    tmp = ReadReg(lib,NRF_REG_OBSERVE_TX);
    observe = *(nrf_reg_observe_tx_t *)&tmp;

#ifdef DEBUG_MODE
    PrintRegister(status); // Assuming PrintRegister is adapted for C.
    printf("Retries count: %d Lost: %d\n", observe.rt_count, observe.plos_count);
#endif

    /* Set result as -1 on fail, and equal to retries on success */
    uint8_t result;
    if (status.max_rt)
    {
        result = -1;
    }
    else
        result = observe.rt_count;

    /* Clear tx_ds and max_rt flags */
    status.rx_dr = 0; // we don't want to clear rx flag
    status.max_rt = 1;
    status.tx_ds = 1;
    WriteReg(lib,NRF_REG_STATUS, _TOUINT(status));

    /* Go to sleep mode */
    CNRFLib_SetSleepTxMode(lib);
    return result;
}

void WriteTxPayload(CNRFLib *lib,uint8_t *pBuff, uint8_t length)
{
    assert(length <= NRF_MAX_PAYLOAD && "Payload width is limited to 32 bytes");

    uint8_t *tmp = (uint8_t *)malloc(NRF_MAX_PAYLOAD);
    memset(tmp, 1, NRF_MAX_PAYLOAD);
    memcpy(tmp, pBuff, length);

    WriteBytes(lib,NRF_CMD_W_TX_PAYLOAD, tmp, NRF_MAX_PAYLOAD);
    free(tmp);
}

void ReadRxPayload(CNRFLib *lib,uint8_t *pBuff, uint8_t length)
{
    assert(length <= NRF_MAX_PAYLOAD && "Payload width is limited to 32 bytes");

    uint8_t *tmp = (uint8_t *)malloc(NRF_MAX_PAYLOAD);
    ReadBytes(lib,NRF_CMD_R_RX_PAYLOAD, tmp, NRF_MAX_PAYLOAD);
    memcpy(pBuff, tmp, length);
    free(tmp);
}

void CNRFLib_Read(CNRFLib *lib, uint8_t *pBuff, uint8_t length)
{
    assert(lib->isAttached && "Call AttachToSpiBus first");
    assert(lib->m_Mode == nrf_rx_mode && "Rx mode must be set");

    /* Since we are in rx mode we already have CE high, power on and PRX bit */

    ReadRxPayload(lib,pBuff, length);

    /* Clear flags */
    nrf_reg_status_t status = CNRFLib_GetStatus(lib);
    status.rx_dr = 1;  // clear
    status.tx_ds = 0;  // don't clear
    status.max_rt = 0; // don't clear
    WriteReg(lib,NRF_REG_STATUS, _TOUINT(status));
}

bool pipe_IsRxDataAvailable(CNRFLib *lib,uint8_t pipeNo)
{
    nrf_reg_status_t status = CNRFLib_GetStatus(lib);
    return status.rx_p_n == pipeNo;
}

bool IsRxDataAvailable(CNRFLib *lib)
{
    nrf_reg_status_t status = CNRFLib_GetStatus(lib);
    return status.rx_dr;
}

void CNRFLib_SetSleepTxMode(CNRFLib *lib)
{
    uint8_t tmp = ReadReg(lib,NRF_REG_CONFIG);
    nrf_reg_config_t cfg = *(nrf_reg_config_t *)&tmp;
    cfg.pwr_up = 0;
    cfg.prim_rx = 0; /* Actually rx mode */
    WriteReg(lib,NRF_REG_CONFIG, _TOUINT(cfg));
    CELow(lib);

    lib->m_Mode = nrf_tx_mode;
}
void SetRxMode(CNRFLib *lib)
{
    uint8_t tmp = ReadReg(lib,NRF_REG_CONFIG);
    nrf_reg_config_t cfg = *(nrf_reg_config_t *)&tmp;
    cfg.pwr_up = 1;
    cfg.prim_rx = 1; /* Actually rx mode */
    WriteReg(lib,NRF_REG_CONFIG, _TOUINT(cfg));
    CEHigh(lib);

    lib->m_Mode = nrf_rx_mode;
    DELAY_MS(2);
}

void CNRFLib_FlushRX(CNRFLib *lib)
{
    WriteBytes(lib,NRF_CMD_FLUSH_RX, NULL, 0);
}

void CNRFLib_FlushTX(CNRFLib *lib)
{
    WriteBytes(lib,NRF_CMD_FLUSH_TX, NULL, 0);
}

nrf_reg_status_t CNRFLib_GetStatus(CNRFLib *lib)
{
    uint8_t tmp = ReadReg(lib,NRF_REG_STATUS);
    nrf_reg_status_t status;
    memcpy((void *)&status, &tmp, sizeof(tmp));
    return status;
}

nrf_reg_fifo_status_t CNRFLib_GetFifoStatus(CNRFLib *lib)
{
    uint8_t tmp = ReadReg(lib,NRF_REG_FIFO_STATUS);
    nrf_reg_fifo_status_t status;
    memcpy((void *)&status, &tmp, sizeof(tmp));
    return status;
}

void SetAutoRetransmission(CNRFLib *lib,uint8_t count, uint8_t delay)
{
    assert(count <= 15 && delay <= 15);

    nrf_reg_retries_t rt;
    rt.count = count;
    rt.delay = delay;
    WriteReg(lib,NRF_REG_SETUP_RETR, _TOUINT(rt));
}

void SetChannel(CNRFLib *lib,uint8_t channel)
{
    assert(channel < 0x80 && "Note that ESP32 is little-endian chip");

    WriteReg(lib,NRF_REG_SETUP_CHANNEL, channel);
}

void SetDataRate(CNRFLib *lib,uint8_t dataRate)
{
    assert(dataRate < 3);

    uint8_t tmp = ReadReg(lib,NRF_REG_SETUP);
    nrf_reg_setup_t setup = *(nrf_reg_setup_t *)&tmp;
    setup.dr_high = (dataRate >> 1) & 1;
    setup.dr_low = dataRate & 1;

#ifdef DEBUG_MODE
    printf("High: %d Low: %d\n", (dataRate >> 1) & 1, dataRate & 1);
#endif

    WriteReg(lib,NRF_REG_SETUP, _TOUINT(setup));
}

void SetPower(CNRFLib *lib,uint8_t power)
{
    assert(power <= 3);

    uint8_t tmp = ReadReg(lib,NRF_REG_SETUP);
    nrf_reg_setup_t setup = *(nrf_reg_setup_t *)&tmp;
    setup.power = power;
    WriteReg(lib,NRF_REG_SETUP, _TOUINT(setup));
}

void SetDataRateAndPower(CNRFLib *lib,uint8_t dataRate, uint8_t power)
{
    assert(dataRate < 3);
    assert(power <= 3);

    uint8_t tmp = ReadReg(lib,NRF_REG_SETUP);
    nrf_reg_setup_t setup = *(nrf_reg_setup_t *)&tmp;
    setup.dr_high = (dataRate >> 1) & 1;
    setup.dr_low = dataRate & 1;
    setup.power = power;
    WriteReg(lib,NRF_REG_SETUP, _TOUINT(setup));
}

void SetPipeAddr(CNRFLib *lib,uint8_t pipeNo, uint8_t *pAddr, uint8_t length)
{
    assert(pipeNo < 6);
    assert(length == (pipeNo < 2 ? 5 : 1));

    WriteBytes(lib,(NRF_CMD_W_REGISTER | NRF_REG_RX_ADDR_P0) + pipeNo, pAddr, length);
}

void GetPipeAddr(CNRFLib *lib,uint8_t pipeNo, uint8_t *pAddr)
{
    assert(pipeNo < 6);

    uint8_t length = pipeNo < 2 ? 5 : 1;
    ReadBytes(lib,(NRF_CMD_R_REGISTER | NRF_REG_RX_ADDR_P0) + pipeNo, pAddr, length);
}

void SetTxAddr(CNRFLib *lib,uint8_t *pAddr, uint8_t length)
{
    assert(length == 5);

    WriteBytes(lib,NRF_CMD_W_REGISTER | NRF_REG_TX_ADDR, pAddr, length);
}
void GetTxAddr(CNRFLib *lib,uint8_t *pAddr)
{
    WriteBytes(lib,NRF_CMD_R_REGISTER | NRF_REG_TX_ADDR, pAddr, 5);
}

void CNRFLib_ScanChannels(CNRFLib *lib, uint64_t *firstHalf, uint64_t *secondHalf)
{
    nrf_mode_t wasMode = lib->m_Mode;
    if (wasMode == nrf_tx_mode)
        SetRxMode(lib);

    for (int i = 0; i < 64; i++)
    {
        SetChannel(lib,i);                        // Assuming SetChannel accepts CNRFLib* and channel as parameters
        uint8_t value = ReadReg(lib,NRF_REG_RPD); // Assuming ReadReg accepts CNRFLib* and reg as parameters
        *firstHalf |= ((uint64_t)value << i);
#ifdef DEBUG_MODE
        printf("Channel: %d value: %d\n", i, value);
#endif
    }

    for (int i = 0; i < 64; i++)
    {
        SetChannel(lib,64 + i);                   // Adjusted for CNRFLib* parameter
        uint8_t value = ReadReg(lib,NRF_REG_RPD); // Adjusted for CNRFLib* parameter
        *secondHalf |= ((uint64_t)value << i);
    }

    if (wasMode == nrf_tx_mode){CNRFLib_SetSleepTxMode(lib);}
    
}

void CELow(CNRFLib *lib)
{
    gpio_set_direction(lib->m_CE, GPIO_MODE_OUTPUT);
    gpio_set_level(lib->m_CE, 0);
}
void CEHigh(CNRFLib *lib)
{
    gpio_set_direction(lib->m_CE, GPIO_MODE_OUTPUT);
    gpio_set_level(lib->m_CE, 1);
}

void CSLow(CNRFLib *lib)
{
    gpio_set_direction(lib->m_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(lib->m_CS, 0);
}
void CSHigh(CNRFLib *lib)
{
    gpio_set_direction(lib->m_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(lib->m_CS, 1);
}

#ifdef DEBUG_MODE
void PrintRegisterConfig(nrf_reg_config_t config)
{
    printf("rx_dr: %d tx_ds: %d max_rt: %d en_crc: %d crco: %d power_up: %d prx: %d\n",
           config.mask_rx_dr, config.mask_tx_ds, config.mask_max_rt, config.en_crc, config.crco, config.pwr_up, config.prim_rx);
}

void PrintRegisterStatus(nrf_reg_status_t status)
{
    printf("tx_full: %d rx_p_no: %d max_rt: %d tx_ds: %d rx_dr: %d\n",
           status.tx_full, status.rx_p_n, status.max_rt, status.tx_ds, status.rx_dr);
}

void PrintRegisterFifoStatus(nrf_reg_fifo_status_t status)
{
    printf("rx_empty: %d rx_full: %d tx_empty: %d tx_full: %d tx_reuse: %d\n",
           status.rx_empty, status.rx_full, status.tx_empty, status.tx_full, status.tx_reuse);
}
#endif /* DEBUG_MODE */
