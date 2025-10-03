TMF8828 Descattering Filter
===========================

Step 0 - setup
--------------

Please find the source code for the de-scattering filter in the same repository / ZIP archive as this document.
You need to include source code and header file into your driver project.

Step 1 - Configure the descattering filter
------------------------------------------

Run:

```C
    descatterConfigure(DESCATTER_THRESHOLD_DEFAULT,DESCATTER_RANGE_MM);
```	

The default threshold is 5, the default range is 10000mm.

If you want to change the threshold later use this:


```C
void setDescatterThreshold ( uint16_t threshold )
{
    descatterConfigure(threshold,DESCATTER_RANGE_MM);
}
```	

Step 2 - Removed false detects from (descatter) results
-------------------------------------------------------

The objects are stored in two arrays:  

  - itsDistance for the distances per zone (max. four objects per zone)
  - itsConfidence for the confidences of these objects
  
  1. reset the descattering filter
  2. add all objects with their matching confidence to the descattering filter data set
  3. check all objects if they have to be removed (confidence to zero)
  
```C
void FrameParser::doDescattering()
{
    descatterConfigure( itsDescatteringThreshold, TMF8XXX_DESCATTER_MAX_DISTANCE_MM );

    for ( int object = 0; object < itsNumberOfPeaks; ++object )
    {
        for ( int row = 0; row < (int)itsRows; ++row )
        {
            for ( int col = 0; col < (int)itsCols; ++col )
            {
                descatteraddObjectPeak( itsDistance[row][col][object], mapConfidence( itsConfidence[row][col][object], false ));
            }
        }
    }

    int removedPeaks = 0;

    for ( int object = 0; object < itsNumberOfPeaks; ++object )
    {
        for ( int row = 0; row < (int)itsRows; ++row )
        {
            for ( int col = 0; col < (int)itsCols; ++col )
            {
                if ( descatterIsScatteringPeak(itsDistance[row][col][object], mapConfidence( itsConfidence[row][col][object], false )) == 1 )
                {
                    itsConfidence[row][col][object] = 0;
                    ++removedPeaks;
                }
            }
        }
    }

    qDebug() << QString::number(removedPeaks) << "objects removed.";
}
```	

Helper function for mapping of a linear confidence value to a logarithmic value:

```C
#define CONF_BREAKPOINT (40)
#define EXP_GROWTH_RATE (1.053676f)

uint16_t FrameParser::mapConfidence ( const uint8_t confidence, const bool limitTo8Bit )
{
    uint32_t exponentialConfidence = 0;

    if (confidence <= CONF_BREAKPOINT)
    {
        exponentialConfidence = confidence;
    }
    else
    {
        /* exponential de-mapping */
        const uint32_t steps = confidence - CONF_BREAKPOINT;
        exponentialConfidence = CONF_BREAKPOINT*pow(EXP_GROWTH_RATE,steps);
    }

    if ( limitTo8Bit )
    {
        exponentialConfidence = ( exponentialConfidence > UINT8_MAX ? UINT8_MAX : exponentialConfidence );
    }
    else
    {
        exponentialConfidence = ( exponentialConfidence > UINT16_MAX ? UINT16_MAX : exponentialConfidence );
    }

    return static_cast<uint16_t>(exponentialConfidence);
}
```	
