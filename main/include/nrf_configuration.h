#ifndef NRF_CONFIG_H
#define NRF_CONFIG_H

// Declare any constants that might be needed externally
#define MAX_RETRY_ATTEMPTS 5
#include "include/mirf.h"

// Function declarations
void receiver(void *pvParameters);
void sender(void *pvParameters);
NRF24_t sender_best_move(int row_coordination, int column_coordination);
void sender_best_move_2(NRF24_t dev,int row_coordination, int column_coordination);

NRF24_t  Nrf_bestMove_config(NRF24_t dev);
NRF24_t Nrf_score_config(NRF24_t dev);
NRF24_t sender_score( uint8_t player_score, uint8_t computer_score);
void sender_score_2( NRF24_t dev,uint8_t player_score, uint8_t computer_score);

#endif // NRF_CONFIG_H
