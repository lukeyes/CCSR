#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "ccsr.h"
#include "utils.h"
#include "sound.h"
#include "irSensors.h"
#include "mood.h"
#include "facial.h"
#include "servoCtrl.h"
#include "mood.h"



extern FILE *logFile;
extern ccsrStateType ccsrState;
extern int devRandom;
extern int pipeFacialMsg[2];
expressionType expr;
int moodDegradationCounter;

char moodLUT[4][4] = {
   { 
      EXPR_ANGRY,
      MOOD_NORMAL,
      MOOD_NORMAL,
      EXPR_HAPPY
   },
   { 
      EXPR_SCARED,
      MOOD_NORMAL,
      MOOD_NORMAL,
      MOOD_NORMAL
   },
   { 
      EXPR_SCARED,
      MOOD_NORMAL,
      MOOD_NORMAL,
      MOOD_NORMAL
   },
   { 
      EXPR_SLEEP,
      MOOD_NORMAL,
      MOOD_NORMAL,
      MOOD_NORMAL
   }
};

void moodInit() {
   ccsrState.happiness    = 0;
   ccsrState.arousal   = 128;
   ccsrState.randomEyeMovements = 1;
   ccsrState.showEmotion = 1;
   ccsrState.blinkRate     = 5;
   ccsrState.eyeMovementRate = 10;
   moodDegradationCounter = MOOD_DEGRADATION_SPEED;
   expr.type = EXPR_LOOKSTRAIGHT;
   write(pipeFacialMsg[IN], &expr,sizeof(expr));
}

void setMood(int h, int a){
   ccsrState.happiness = ccsrState.happiness + h;
   ccsrState.arousal = ccsrState.arousal + a;
   if(ccsrState.happiness > MAX_HAPPINESS){
      ccsrState.happiness = MAX_HAPPINESS;
   }
   else if(ccsrState.happiness < MIN_HAPPINESS){
      ccsrState.happiness = MIN_HAPPINESS;
   }
   if(ccsrState.arousal > MAX_AROUSAL){
      ccsrState.arousal = MAX_AROUSAL;
   }
   else if(ccsrState.arousal < MIN_AROUSAL){
      ccsrState.arousal = MIN_AROUSAL;
   }
}

void *mood() {
   char random;
   int blinkCount;
   int eyeMovementCount;
   expressionType expr;
   float r, g, b, h, v;
   int R, G, B;
   char xLUT, yLUT;

   logMsg(logFile, "Starting mood maintenance", LOG); 
   blinkCount = ccsrState.blinkRate;
   eyeMovementCount = ccsrState.eyeMovementRate;

   while(1) {
 //     printf("semot %d\n", ccsrState.showEmotion);

      if(ccsrState.happiness > MIN_HAPPINESS) {
         ccsrState.happiness  = ccsrState.happiness - 1;
      }
      if(ccsrState.arousal > MIN_AROUSAL) {
         ccsrState.arousal = ccsrState.arousal - 1;
      }


      v = 100*ccsrState.arousal/MAX_AROUSAL;
      h = 280 - ( (280*(ccsrState.happiness + MAX_HAPPINESS))/(2*MAX_HAPPINESS));

      HVtoRGB( &r, &g, &b, h, v );

      R = (int) 255*r/255;
      G = (int) 255*g/255;
      B = (int) 255*b/255;
//      printf("mood RGB %d %d %d\n", R, G, B);

      if(ccsrState.showEmotion){
         setRGBLED(R, G, B, 90);
      }

      xLUT = 2*(ccsrState.happiness + MAX_HAPPINESS)/MAX_HAPPINESS;
      yLUT = 3 - 4*ccsrState.arousal/MAX_AROUSAL;
//      printf("mood hap %d ar %d X %d Y %d rgb %f %f %f RGB %d %d %d HV %f %f \n", ccsrState.happiness, ccsrState.arousal, xLUT, yLUT, r, g, b, R, G, B, h, v);
      if(moodLUT[yLUT][xLUT] != MOOD_NORMAL){
         if(ccsrState.showEmotion){
            if(moodLUT[yLUT][xLUT] == EXPR_SLEEP){
               if(ccsrState.state != SM_SLEEP) {
                  goToSleep();
               }
            }
            else {
               if(ccsrState.state == SM_SLEEP) {
                  wakeFromSleep();
               }
               expr.type = moodLUT[yLUT][xLUT];
               write(pipeFacialMsg[IN], &expr,sizeof(expr));
            }
         }
      }
      else {
         if(ccsrState.state == SM_SLEEP) {
            wakeFromSleep();
         }
         if(ccsrState.randomEyeMovements){
            if(eyeMovementCount==0){
               if(read(devRandom, &random, 1) != 1) {
                  logMsg(logFile, "Unsuccessful read from /dev/random", ERROR);	   
               }
               eyeMovementCount = (ccsrState.eyeMovementRate * abs(random)/128);
               if(read(devRandom, &random, 1) != 1) {
                  logMsg(logFile, "Unsuccessful read from /dev/random", ERROR);	   
               }
               expr.type = ((EXPR_LOOKDOWN - EXPR_LOOKSTRAIGHT) * abs(random)/128) + EXPR_LOOKSTRAIGHT;
               write(pipeFacialMsg[IN], &expr,sizeof(expr));
            }
            else{
               eyeMovementCount = eyeMovementCount - 1;
            }
         }
      }
      if(ccsrState.randomEyeMovements){
         if(blinkCount==0){
            expr.type = EXPR_BLINK;
            write(pipeFacialMsg[IN], &expr,sizeof(expr));
            if(read(devRandom, &random, 1) != 1) {
               logMsg(logFile, "Unsuccessful read from /dev/random", ERROR);	   
            }
            blinkCount = (ccsrState.blinkRate * abs(random)/128);
         }
         else{
            blinkCount = blinkCount - 1;
         }
      }
      usleep(MOOD_PERIOD);
   }
}

// HSVtoRGB, but always S=1: fully saturated 
void HVtoRGB( float *r, float *g, float *b, float h, float v )
{
	int i;
	float f, p, q, t;
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = 0;
	q = v * ( 1 -  f );
	t = v * ( 1 - ( 1 - f ) );
	switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}

