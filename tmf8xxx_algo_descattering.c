/*
 *****************************************************************************
 * Copyright by ams OSRAM AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */


/*! \file tmf8xxx_algo_descattering.c
 *
 * \brief descattering filter, also known as ghost filter aka inter-lens-reflection filter.
*/

/*
 *****************************************************************************
 * INCLUDES
 *****************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#include <stdio.h>
#endif
#include "tmf8xxx_algo_descattering.h"

/*
 *****************************************************************************
 * FUNCTIONS
 *****************************************************************************
 */

static tmf8xxxDescatterData   data;
static tmf8xxxDescatterConfig config;

void descatterConfigure ( uint8_t thresholdPercent, uint16_t maxScatterDistMm )
{
    config.maxScatterDistMm = maxScatterDistMm;
    /* Convert 0..99% to 0.0 .. 0.99 [UQ16]. */
    thresholdPercent = thresholdPercent <= 99 ? thresholdPercent : 99;
    config.threshUQ16 = thresholdPercent * ( (1UL<<16) / 100 );

    for ( uint32_t bin = 0; bin < TMF8XXX_DESCATTER_FILTER_LENGTH; ++bin )
    {
        data.bins[bin] = 0;
    }

#ifdef DEBUG
    printf("[FILTERLIB-descatterConfigure] Threshold: %d, MaxDistance: %d\n",thresholdPercent,maxScatterDistMm);
#endif
}

void descatterReset ( )
{
    for ( uint32_t bin = 0; bin < TMF8XXX_DESCATTER_FILTER_LENGTH; ++bin )
    {
        data.bins[bin] = 0;
    }

#ifdef DEBUG
    printf("[FILTERLIB-descatterReset]\n");
#endif
}

void descatteraddObjectPeak ( uint16_t distanceMm, uint16_t confidence )
{
    uint16_t* bin;
    int16_t   firstIdx;
    int16_t   lastIdx;
    uint16_t  scaledVal;

#ifdef DEBUG
    printf("[FILTERLIB-descatteraddObjectPeak] Distance: %d, Confidence: %d\n",distanceMm,confidence);
#endif

    if ( confidence == 0 )
    {
        return; /* Invalid object, skip. */
    }

    firstIdx = ( distanceMm - config.maxScatterDistMm ) / TMF8XXX_DESCATTER_BIN_WIDTH_MM;
    lastIdx  = ( distanceMm + config.maxScatterDistMm ) / TMF8XXX_DESCATTER_BIN_WIDTH_MM;
    /* This is a linear threshold. Change if your metric is different.*/
    scaledVal = ( (uint32_t)confidence * config.threshUQ16 + (1<<15) ) >> 16;

    if ( firstIdx < 0 ) /* Clip to filter length */
    {
        firstIdx = 0;
    }
    if ( lastIdx > (int16_t)( TMF8XXX_DESCATTER_FILTER_LENGTH - 1 ) )
    {
        lastIdx = TMF8XXX_DESCATTER_FILTER_LENGTH - 1;
    }

    for ( bin = &data.bins[ firstIdx ]; bin < &data.bins[ lastIdx ]; ++bin )
    {
        if( *bin < scaledVal )
        {
            *bin = scaledVal;
        }
    }
}

int descatterIsScatteringPeak ( uint16_t distanceMm, uint16_t confidence )
{
#ifdef DEBUG
    printf("[FILTERLIB-descatterIsScatteringPeak] Distance: %d, Confidence: %d\n",distanceMm,confidence);
#endif

    if ( distanceMm >= TMF8XXX_DESCATTER_MAX_DISTANCE_MM )
    {
        return 0;
    }

    uint32_t index = distanceMm / TMF8XXX_DESCATTER_BIN_WIDTH_MM;
    return  ( confidence <= data.bins[ index ] );
}

#ifdef __cplusplus
}
#endif
