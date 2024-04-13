#ifndef NRF_CONFIG_H
#define NRF_CONFIG_H

// Declare any constants that might be needed externally
#define MAX_RETRY_ATTEMPTS 8
#include "include/mirf.h"

// Function declarations

void sender(void *pvParameters);
void sender_best_move(NRF24_t dev,int row_coordination, int column_coordination);
NRF24_t  Nrf_bestMove_config(NRF24_t dev);
NRF24_t Nrf_score_config(NRF24_t dev);
void sender_score(NRF24_t dev,uint8_t player_score, uint8_t computer_score);

#endif // NRF_CONFIG_H
