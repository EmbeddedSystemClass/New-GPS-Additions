/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * dkgps.c - ublox max m8c pico breakout with chip antenna driver for deck port.
 */
#define DEBUG_MODULE "DKGPS"

#include <stdint.h>
#include <string.h>
//#include <stdio.h> //conflict with stabilizer_types.h via #include "compass.h"

#include "stm32fxxx.h"
#include "config.h"
#include "console.h"
#include "uart1.h"
#include "debug.h"
#include "deck.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "log.h"

#define ENABLE_GPS_DECK

/**
 * dkgps.h is needed for the Position Hold capability
 * and when dkgps.c is called by sensors_stock.c to pick up
 * 3D fix positions
 */ 

#include "dkgps.h"

#if defined(COMPASS_ENABLED)
/**
 * compass.h is needed for the Position Hold Mode capability
 * and when dkgps.c calls compass.c to see if is calibrated
 */  

#include "compass.h"
#endif 

static bool isInit;

// Default is NMEA, 9600 baudrate and update rate at 1 Hz
// Operating mode is ubx binary, 115200 baudrate and update rate at 5 Hz 
// SBAS Mode enabled, DGPS Mode enabled

typedef struct {
  uint32_t itow;
  int32_t ftow;
  int16_t week;
  uint8_t fixType;
  uint8_t fixStatus;
  uint8_t fixMode;
  int32_t ecefx;
  int32_t ecefy;
  int32_t ecefz;
  uint32_t pacc;
  int32_t ecefvx;
  int32_t ecefvy;
  int32_t ecefvz;
  uint32_t sacc;
  uint16_t pdop;
  uint8_t reserved1;
  uint8_t numSat;
  uint32_t reserved2;
  int32_t lon;
  int32_t lat;
  int32_t height;
  int32_t hMSL;
  uint32_t hAcc;
  uint32_t vacc;
  uint32_t temp32;
  uint16_t temp16; 
} MeasData;

struct nav_sol {
  uint32_t itow;
  int32_t ftow;
  int16_t week;
  uint8_t fixType;
  uint8_t fixStatus;
  int32_t ecefx;
  int32_t ecefy;
  int32_t ecefz;
  uint32_t pacc;
  int32_t ecefvx;
  int32_t ecefvy;
  int32_t ecefvz;
  uint32_t sacc;
  uint16_t pdop;
  uint8_t reserved1;
  uint8_t numSat;
  uint32_t reserved2;
} __attribute__ ((packed));

struct nav_posllh {
  uint32_t itow;
  int32_t lon;
  int32_t lat;
  int32_t height;
  int32_t hMSL;
  uint32_t hAcc;
  uint32_t vacc;
} __attribute__ ((packed));

struct ubxbin_message {
    union {
        struct {
            uint8_t class;
            uint8_t id;
        } __attribute__ ((packed));
        uint16_t class_id;
    } __attribute__ ((packed));
    uint16_t len;
    union {
    	void * payload;
    	struct nav_sol* nav_sol;
    	struct nav_posllh* nav_posllh;
    };
    union {
        struct {
            uint8_t ck_a;
            uint8_t ck_b;
        } __attribute__ ((packed));
        uint16_t ck;
    };
} __attribute__ ((packed));

static MeasData m;
#define NAV_SOL 0x0601
#define NAV_POSLLH 0x0201

static bool     gps_newFrame = false;
static bool     gps_newPOSLLH = false;
static bool     gps_newSOL = false;
static float    gps_hMSL;
static float    gps_hAcc;
static float    gps_pAcc;
static float    gps_scaleLat = 0.011113295f; //degrees * 1e+7 convert to meters
static float    gps_scaleLon;
static float    D2R;
static bool     gps_setHome = false;
static int32_t  gps_latHome = 0;
static int32_t  gps_lonHome = 0;

static uint32_t timestamp = false;
static float pos_x = 0.0f;
static float pos_y = 0.0f;
static float pos_z = 0.0f;

void dkgpsGetFrameData(uint32_t* ts, float* px, float* py, float* pz)
{
  *ts = timestamp;
  *px = pos_x;
  *py = pos_y;
  *pz = pos_z;
}

static void saveFrameData(void)
{
  static float phi;
    
//Assume 3D Fix or 3D Fix/DGPS    
  if ((m.fixType > 2) && (m.numSat >= 5) && (gps_hAcc <= 6.0f))
  {
    if (!gps_setHome)  
    {
      gps_latHome = m.lat;   //deg * 1e+7
      gps_lonHome = m.lon;   //deg * 1e+7
      gps_setHome = true;
      phi = m.lat / 10000000.0f * D2R;
      gps_scaleLon = cosf(phi) * gps_scaleLat;    
    }
    
//Assume right-hand xyz cartesian coordinate system
//Assume x = latitude pointing to North geographic or plus latitude  
//Assume y = longitude pointing to West geographic or minus longitude
//Assume z = vertical pointing downward or minus hMSL
//Assume yawgeo is +/- 180 degrees and + is counterclockwise
//Assume yawgeo in degrees is available to map xyz to level cf1/cf2 fwd direction   
//Preserve 1 cm significance by using Home position offset
//don't output positions until compass is calibrated
    
#if defined(COMPASS_ENABLED) 
    else if (compassCaled())
    {
      timestamp = 1;  
      pos_x = (m.lat - gps_latHome) * gps_scaleLat;   //meters
      pos_y = -(m.lon - gps_lonHome) * gps_scaleLon;  //meters
      pos_z = -gps_hMSL;                              //meters
    }
#endif    
  }  
  else
  {
    timestamp = 0;     
    pos_x = 0.0f;
    pos_y = 0.0f;
    pos_z = 0.0f;
  }

//This code can be found in position_controller_pid.c
//  In converting position (desired-measured) to roll/pitch
//  D2R = (float) M_PI/180.0f
//  cos = cosf(yawgeo * D2R)
//  sin = sinf(yawgeo * D2R)
//  pitch = - position.x * cos - position.y * sin
//  roll  = - position.y * cos + position.x * sin
  
}

static char dkgpsGetc()
{
  char c;
  uart1Getchar(&c);
  return c;
}

static void dkgpsRead(void *buffer, int length)
{
  int i;

  for (i=0; i<length; i++)
  {
    ((char*)buffer)[i] = dkgpsGetc();
  }
}

static void dkgpsReceiveBin(struct ubxbin_message* msg, int maxPayload)
{
  while (!gps_newFrame)
  {
    if ((uint8_t)dkgpsGetc() != 0xB5)
        continue;
    if ((uint8_t)dkgpsGetc() != 0x62)
        continue;

    msg->class = dkgpsGetc();
    msg->id = dkgpsGetc();
    dkgpsRead(&msg->len, 2);

    if(msg->len > maxPayload)
      continue;

    dkgpsRead(msg->payload, msg->len);
    msg->ck_a = dkgpsGetc();
    msg->ck_b = dkgpsGetc();

    gps_newFrame = true;
  }    
}

void dkgpsRxTask(void *param)
{
  struct ubxbin_message msg;
  char payload[100];
  msg.payload = payload;

//  const char raw_0_stopnmea[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA9};
  const char raw_1_baudrate[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBE, 0x72};
  const char raw_2_enwaas[]   = {0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x03, 0x07, 0x03, 0x00, 0x04, 0xE0, 0x04, 0x00, 0x19, 0x9D};
  const char raw_3_ensol[]    = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x17, 0xDA};
  const char raw_4_enposllh[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x13, 0xBE};
  const char raw_5_en5hz[]    = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A};
  
//  uart1SendData(sizeof(raw_0_stopnmea), (uint8_t*)raw_0_stopnmea);
//  vTaskDelay(1000);

  uart1SendData(sizeof(raw_1_baudrate), (uint8_t*)raw_1_baudrate);  //changes baudrate & stops NMEA
  vTaskDelay(1000);
  
  uart1Init(115200); 

  vTaskDelay(2000);
  uart1SendData(sizeof(raw_2_enwaas), (uint8_t*)raw_2_enwaas);

  vTaskDelay(1000);
  uart1SendData(sizeof(raw_3_ensol), (uint8_t*)raw_3_ensol);

  vTaskDelay(1000);
  uart1SendData(sizeof(raw_4_enposllh), (uint8_t*)raw_4_enposllh);

  vTaskDelay(1000);
  uart1SendData(sizeof(raw_5_en5hz), (uint8_t*)raw_5_en5hz);
  vTaskDelay(1000);

  while(1)
  {
    dkgpsReceiveBin(&msg, 100);

    if ((msg.class_id == NAV_SOL) && !gps_newSOL) {

      m.fixType= msg.nav_sol->fixType; 
      m.fixStatus = msg.nav_sol->fixStatus;
      m.numSat = msg.nav_sol->numSat;
      m.itow = msg.nav_sol->itow;
      m.pacc = msg.nav_sol->pacc;
      gps_pAcc = m.pacc / 1000.0;   
      m.fixStatus &= 1;
      gps_newSOL = true;
    }

    if ((msg.class_id == NAV_POSLLH) && !gps_newPOSLLH) {

      m.lat = msg.nav_posllh->lat;
      m.lon = msg.nav_posllh->lon;
      m.hMSL = msg.nav_posllh->hMSL;
      gps_hMSL = m.hMSL / 1000.0;
      m.hAcc = msg.nav_posllh->hAcc;
      gps_hAcc = m.hAcc / 1000.0;
      gps_newPOSLLH = true;      
    }
    if (gps_newPOSLLH && gps_newSOL) {
      if(m.fixStatus == 1)
      {
        if(m.fixType == 2)
        {
          m.fixType = 6;
        }
        else  if(m.fixType == 3)
        {
          m.fixType = 7;
        }
        m.fixMode = 2;
      }
      else
      {
        if(m.fixType) m.fixMode = 1; else m.fixMode = 0;
      }
      saveFrameData();
      gps_newSOL = false;
      gps_newPOSLLH = false;
    }
  gps_newFrame = false;  
  }
}  
 
static void dkgpsInit(DeckInfo *info)
{
  if(isInit)
    return;

  DEBUG_PRINT("Enabling reading from ublox GPS\n");

  uart1Init(9600);

  xTaskCreate(dkgpsRxTask, "DKGPS",
              configMINIMAL_STACK_SIZE, NULL, /*priority*/1, NULL);

  D2R = (float) M_PI/180.0;
  isInit = true;
}

static bool dkgpsTest()
{
  bool status = true;

  if(!isInit)
    return false;

  return status;
}

static const DeckDriver dkgps_deck = {
  .vid = 0xBC,             //prototype .vid value selection 
  .pid = 0x07,             //prototype .pid value selection 
  .name = "bcDKGPS",

  .usedPeriph = 0,
  .usedGpio = DECK_USING_PA2 | DECK_USING_PA3,

  .init = dkgpsInit,
  .test = dkgpsTest,
};

#ifdef ENABLE_GPS_DECK
DECK_DRIVER(dkgps_deck);
#endif

LOG_GROUP_START(gps)
LOG_ADD(LOG_INT32, lat, &m.lat)             //degrees * 1e+7
LOG_ADD(LOG_INT32, lon, &m.lon)             //degrees * 1e+7
LOG_ADD(LOG_FLOAT, hMSL, &gps_hMSL)         //meters
LOG_ADD(LOG_FLOAT, hAcc, &gps_hAcc)         //meters
LOG_ADD(LOG_UINT8, nsat, &m.numSat)
LOG_ADD(LOG_INT32, fix, &m.fixMode)         //0-no DGPS, 1-DGPS
LOG_ADD(LOG_FLOAT, pAcc, &gps_pAcc)         //meters
LOG_ADD(LOG_UINT8, fixType, &m.fixType)     //NMEA 0183 Protocol (0,2,3,6,7)
LOG_ADD(LOG_UINT8, fixStatus, &m.fixStatus) //0-no, 1-fix, 2-dgps
LOG_ADD(LOG_UINT32,fixtime, &m.itow)
LOG_GROUP_STOP(gps)

LOG_GROUP_START(gps_dpos)
LOG_ADD(LOG_UINT32, timestamp, &timestamp)
LOG_ADD(LOG_FLOAT, pos_x, &pos_x)
LOG_ADD(LOG_FLOAT, pos_y, &pos_y)
LOG_ADD(LOG_FLOAT, pos_z, &pos_z)
LOG_GROUP_STOP(gps_dpos)
