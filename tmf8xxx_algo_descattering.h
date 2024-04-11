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

/*! \file tmf8xxx_algo_descattering.h
 *
 * \brief descattering filter - also known as ghost filter aka inter-lens-reflection filter.
 */

#ifndef TMF8XXX_ALGO_DESCATTERING_H
#define TMF8XXX_ALGO_DESCATTERING_H

/*
 *****************************************************************************
 * INCLUDES
 *****************************************************************************
 */
#include <stdint.h>

/*
 *****************************************************************************
 * DEFINES
 *****************************************************************************
 */
/*! The maximum distance at which de-scattering shall be done. */
#define TMF8XXX_DESCATTER_MAX_DISTANCE_MM 10000UL /* = 10m */
/*! The descattering resolution. Every object is sorted into an array of this resolution.
 *  2**N-values are recommended, as this this is used as divisor.*/
#define TMF8XXX_DESCATTER_BIN_WIDTH_MM 8UL /* = 8mm descattering data resolution. */

/*! The descatter filter length. */
#define TMF8XXX_DESCATTER_FILTER_LENGTH ( TMF8XXX_DESCATTER_MAX_DISTANCE_MM / TMF8XXX_DESCATTER_BIN_WIDTH_MM )

/*
 *****************************************************************************
 * TYPES
 *****************************************************************************
 */

/*! The configuration for the descattering filter. Can be read-only. */
typedef struct _tmf8xxxDescatterConfig
{
    uint16_t threshUQ16; /*!< Objects are considered scatters if they're below the threshold (0 .. 99% = 0.0 .. 0.99 UQ16). */
    uint8_t  maxScatterDistMm; /*!< Defines how far a scattering peak can be away from the origin object (+/-). */
} tmf8xxxDescatterConfig;


/*! The descattering filter data. */
typedef struct _tmf8xxxDescatterData
{
    uint16_t bins[ TMF8XXX_DESCATTER_FILTER_LENGTH ]; /*!< The filter data. Every bin is an object distance range. */
} tmf8xxxDescatterData;

/*
 *****************************************************************************
 * FUNCTIONS
 *****************************************************************************
 */

/*! \brief Initialize the descattering filter with a certain configuration.
 *         Call this function every time the config changes.
 *  \param thresholdPercent The scattering threshold.
 *  \param maxScatterDistMm The maximum distance between a peak and it's scatter object (without distance correction).
 */
void descatterConfigure( uint8_t thresholdPercent, uint16_t maxScatterDistMm );

/*! \brief Reset the learned scatter filter data.
 *         Call this function every time you want to process a new result set.
 */
void descatterReset( );

/*! \brief Add a peak to the descatter filter.
 *         Call this function for every un-filtered peak to "train" the filter.
 *  \param distanceMm The object/descatter distance in mm.
 *  \param val        The object value (SNR/weight/signal/...).
 */
void descatteraddObjectPeak( uint16_t distanceMm, uint16_t confidence );

/*! \brief Check if a peak is a scatter peak.
 *         Call this function after all peaks of the frame has been trained
 *         for every un-filtered object peak to filter ghosts.
 *  \param distanceMm The object/descatter distance in mm.
 *  \param val        The object value (SNR/weight/signal/...).
 *  \return 1 if the peak is a scatter peak -> can be removed from results.
 */
int descatterIsScatteringPeak( uint16_t distanceMm, uint16_t val );

#endif /* TMF8XXX_ALGO_DESCATTERING_H */
