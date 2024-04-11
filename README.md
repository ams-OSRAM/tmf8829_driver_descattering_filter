TMF8828 Descattering Filter
===========================

Step 0 - setup
--------------

Please find the source code for the de-scattering filter in the same ZIP archive as this document.
You need to include source code and header file into your driver project.

Step 1 - Configure the descattering filter
------------------------------------------

Run:

```C
    descatterConfigure(DESCATTER_THRESHOLD_DEFAULT,DESCATTER_RANGE_MM);
```	

The default threshold is 5, the default range is 5000mm.

If you want to change the threshold later use this:


```C
void setDescatterThreshold ( uint16_t threshold )
{
    descatterConfigure(threshold,DESCATTER_RANGE_MM);
}
```	

Step 2 - Removed false detects from (descatter) results
-------------------------------------------------------

The objects are stored in four arrays:  

  - itsObject0Distance / itsObject1Distance for the distances per zone (max. two objects per zone)
  - itsObject1Confidence / itsObject1Confidence for the confidences of these objects
  
  1. reset the descattering filter
  2. add all objects with their matching confidence to the descattering filter data set
  3. check all objects if they have to be removed (confidence to zero)
  
```C
void descatterResults ( void )
{
    descatterReset();

    for ( uint32_t currentPixel = 0; currentPixel < TMF8820C_NUMBER_OF_PIXELS; ++currentPixel )
    {
        descatteraddObjectPeak(itsObject0Distance[currentPixel],mapConfidence(itsObject0Confidence[currentPixel]));
        descatteraddObjectPeak(itsObject1Distance[currentPixel],mapConfidence(itsObject1Confidence[currentPixel]));
    }

    for ( uint32_t currentPixel = 0; currentPixel < TMF8820C_NUMBER_OF_PIXELS; ++currentPixel )
    {
        if ( descatterIsScatteringPeak(itsObject0Distance[currentPixel],mapConfidence(itsObject0Confidence[currentPixel])))
        {
            itsObject0Confidence[currentPixel] = 0;
        }
        if ( descatterIsScatteringPeak(itsObject1Distance[currentPixel],mapConfidence(itsObject1Confidence[currentPixel])))
        {
            itsObject1Confidence[currentPixel] = 0;
        }
    }
}  
```	

Helper function for mapping of a linear confidence value to a logarithmic value:

```C
#define CONF_BREAKPOINT (40)
#define EXP_GROWTH_RATE (1.053676f)

uint16_t mapConfidence ( uint16_t confidence )
{
    uint16_t exp_conf = 0;

    if (confidence <= CONF_BREAKPOINT)
    {
        exp_conf = confidence;
    }
    else
    {
        /* exponential de-mapping */
        const uint16_t steps = confidence - CONF_BREAKPOINT;
        exp_conf = CONF_BREAKPOINT*pow(EXP_GROWTH_RATE,steps);
    }

    return exp_conf;
}
```	
