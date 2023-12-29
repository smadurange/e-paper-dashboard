#ifndef DHT_H
#define DHT_H

void dht_init(void);

void dht_temp_get(char s[7]);

void dht_humid_get(char s[6]);

#endif /* DHT_H */
