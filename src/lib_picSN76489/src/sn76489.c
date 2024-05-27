/*******************************************************************************
 * @file    sn76489.c
 * @author  Jay Convertino(electrobs@gmail.com)
 * @date    2022.01.23
 * @brief   Library for SN76489 sound chip.
 * 
 * @license mit
 * 
 * Copyright 2022 Johnathan Convertino
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ******************************************************************************/

#include <xc.h>
#include <stdint.h>

#include <sn76489.h>

/** DEFINES **/
#define VOICE1_FREQ 0
#define VOICE2_FREQ 2
#define VOICE3_FREQ 4
#define VOICE1_ATTN 1
#define VOICE2_ATTN 3
#define VOICE3_ATTN 5
#define NOISE_CTRL  6
#define NOISE_ATTN  7
#define FIRST_BYTE  0x80
#define SECOND_BYTE 0x00
#define REG_SHIFT   4

/** SEE MY PRIVATES **/
/*** send data to port and control chip ***/
void sendData(struct s_sn76489 *p_sn76489, uint8_t data);

/** INITIALIZE AND FREE MY STRUCTS **/

/*** Initialize sn76489 tris port direction and struct pins ***/
void initSN76489port(struct s_sn76489 *p_sn76489, volatile unsigned char *p_dataTRIS, volatile unsigned char *p_ctrlTRIS, volatile unsigned char *p_readyTRIS, uint8_t nWE, uint8_t nCE, uint8_t ready)
{
  if(!p_dataTRIS) return;
  
  if(!p_ctrlTRIS) return;
  
  if(!p_readyTRIS) return;
  
  if(!p_sn76489) return;
  
  /**** All outputs for data port ****/
  *p_dataTRIS = 0;
  
  /**** nWE is output, nCE is output ****/
  *p_ctrlTRIS &= ~(((unsigned)1 << nWE) | ((unsigned)1 << nCE));
  
  /**** ready is input ****/
  *p_readyTRIS |= ((unsigned)1 << ready);
  
  /**** setup struct with pin locations ****/
  p_sn76489->nWE = nWE;
  
  p_sn76489->nCE = nCE;
  
  p_sn76489->ready = ready;
}

/*** Initialize sn76489 struct ports ***/
void initSN76489(struct s_sn76489 *p_sn76489, volatile unsigned char *p_dataPortW, volatile unsigned char *p_ctrlPortW, volatile unsigned char *p_readyPortR)
{
  if(!p_dataPortW) return;
  
  if(!p_ctrlPortW) return;
  
  if(!p_readyPortR) return;
  
  if(!p_sn76489)  return;
  
  /**** setup ports for library usage ****/
  p_sn76489->p_dataPortW = p_dataPortW;
  
  p_sn76489->p_readyPortR = p_readyPortR;
  
  p_sn76489->p_ctrlPortW = p_ctrlPortW;
  
  *(p_sn76489->p_ctrlPortW) = ~0;
  
  *(p_sn76489->p_dataPortW) = 0;
  
  /**** mute all ****/
  
  setSN76489voice_attn(p_sn76489, 1, 15);
  
  setSN76489voice_attn(p_sn76489, 2, 15);
  
  setSN76489voice_attn(p_sn76489, 3, 15);
  
  setSN76489noise_attn(p_sn76489, 15);
}

/*** calculate freqDiv ***/
uint16_t getSN76489_FreqDiv(uint32_t refClk, uint32_t voiceFreq)
{
  return (refClk/(voiceFreq << 5)) & 0x03FF;
}

/** SET YOUR DATA **/

/*** set sn76489 voice frequency ***/
void setSN76489voice_freq(struct s_sn76489 *p_sn76489, uint8_t voice, uint16_t freqDiv)
{
  uint8_t regVoice = 0;
  
  /**** NULL check ****/
  if(!p_sn76489) return;
  
  /**** Select the correct register for the selected voice ****/
  switch(voice)
  {
    case 1:
      regVoice = VOICE1_FREQ;
      break;
    case 2:
      regVoice = VOICE2_FREQ;
      break;
    case 3:
      regVoice = VOICE3_FREQ;
      break;
    default:
      regVoice = VOICE1_FREQ;
      break;
  }
  
  sendData(p_sn76489, (((unsigned)regVoice << REG_SHIFT) | (freqDiv & 0x000F) | FIRST_BYTE));
  sendData(p_sn76489, (((freqDiv & 0x03F0) >> 4) | SECOND_BYTE));
}

/*** set sn76489 voice attenuation ***/
void setSN76489voice_attn(struct s_sn76489 *p_sn76489, uint8_t voice, uint8_t attenuate)
{
  uint8_t regVoice = 0;
  
  /**** NULL check ****/
  if(!p_sn76489) return;
  
  /**** Select the correct register for the selected voice ****/
  switch(voice)
  {
    case 1:
      regVoice = VOICE1_ATTN;
      break;
    case 2:
      regVoice = VOICE2_ATTN;
      break;
    case 3:
      regVoice = VOICE3_ATTN;
      break;
    default:
      regVoice = VOICE1_ATTN;
      break;
  }
  
  sendData(p_sn76489, ((unsigned)regVoice << REG_SHIFT) | attenuate | FIRST_BYTE);
}

/*** set sn76489 noise attenuation ***/
void setSN76489noise_attn(struct s_sn76489 *p_sn76489, uint8_t attenuate)
{
  /**** NULL check ****/
  if(!p_sn76489) return;
  
  sendData(p_sn76489, ((unsigned)NOISE_ATTN << REG_SHIFT) | attenuate | FIRST_BYTE);
}

/*** set sn76489 noise controls***/
void setSN76489noiseCtrl(struct s_sn76489 *p_sn76489, uint8_t type, uint8_t rate)
{
  /**** NULL check ****/
  if(!p_sn76489) return;
  
  sendData(p_sn76489, ((unsigned)NOISE_CTRL << REG_SHIFT) | (unsigned)rate | ((unsigned)type << 2) | FIRST_BYTE);
}

/*** send data to chip ***/
void sendData(struct s_sn76489 *p_sn76489, uint8_t data)
{
  /**** NULL check ****/
  if(!p_sn76489) return;
  
  di();
  
  /**** make sure we are ready for data ****/
  /**** this method is very slow, only reason I am using it is its portable.
   *    optimize this section to gain in speed with assembler or pin change irq.
   ****/
  while(!((*(p_sn76489->p_readyPortR) >> p_sn76489->ready) & 0x01));
  
  *(p_sn76489->p_dataPortW) = data;
  
  *(p_sn76489->p_ctrlPortW) &= ~((unsigned)1 << p_sn76489->nCE) & ~((unsigned)1 << p_sn76489->nWE);;
  
  while(!((*(p_sn76489->p_readyPortR) >> p_sn76489->ready) & 0x01));
  
  *(p_sn76489->p_ctrlPortW) |= ((unsigned)1 << p_sn76489->nCE) | ((unsigned)1 << p_sn76489->nWE);
  
  ei();
}
