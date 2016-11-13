/*
  ==============================================================================

    WaveshapeProgram.h
    Created: 12 Nov 2016 9:28:17am
    Author:  Edward Davies

  ==============================================================================
*/

#ifndef WAVESHAPEPROGRAM_H_INCLUDED
#define WAVESHAPEPROGRAM_H_INCLUDED

class WaveshapeProgram : public LEDGrid::Program
{
public:
    WaveshapeProgram (LEDGrid& lg) : Program (lg)
    {
    }

    void generateWaveshapes()
    {
        uint8 sineWaveY[45];
        uint8 squareWaveY[45];
        uint8 sawWaveY[45];
        uint8 triangleWaveY[45];

        // Set current phase position to 0 and work out the required phase increment for one cycle
        double currentPhase = 0.0;
        double phaseInc = (1.0 / 30.0) * (2.0 * double_Pi);

        for (int x = 0; x < 30; ++x)
        {
            // Scale and offset the sin output to the Lightpad display
            double sineOutput = sin (currentPhase);
            sineWaveY[x] = roundToInt ((sineOutput * 6.5) + 7.0);

            // Square wave output, set flags for when vertical line should be drawn
            if (currentPhase < double_Pi)
            {
                if (x == 0)
                    squareWaveY[x] = 20;
                else
                    squareWaveY[x] = 1;
            }
            else
            {
                if (squareWaveY[x - 1] == 1)
                    squareWaveY[x - 1] = 20;

                squareWaveY[x] = 13;
            }

            // Saw wave output, set flags for when vertical line should be drawn
            sawWaveY[x] = 14 - ((x / 2) % 15);

            if (sawWaveY[x] == 0 && sawWaveY[x - 1] != 20)
                sawWaveY[x] = 20;

            // Triangle wave output
            triangleWaveY[x] = x < 15 ? x : 14 - (x % 15);

            // Add half cycle to end of array so it loops correctly
            if (x < 15)
            {
                sineWaveY[x + 30] = sineWaveY[x];
                squareWaveY[x + 30] = squareWaveY[x];
                sawWaveY[x + 30] = sawWaveY[x];
                triangleWaveY[x + 30] = triangleWaveY[x];
            }

            // Increment the current phase
            currentPhase += phaseInc;
        }

        for (int i = 0; i < 45; ++i)
        {
            ledGrid.setDataByte (sineWaveOffset + i, sineWaveY[i]);
            int sineByte = ledGrid.getDataByte (sineWaveOffset + i);
            jassert (sineByte == sineWaveY[i]);

            ledGrid.setDataByte (squareWaveOffset + i, squareWaveY[i]);
            int squareByte = ledGrid.getDataByte (squareWaveOffset + i);
            jassert (squareByte == squareWaveY[i]);

            ledGrid.setDataByte (sawWaveOffset + i, sawWaveY[i]);
            int sawByte = ledGrid.getDataByte (sawWaveOffset + i);
            jassert (sawByte == sawWaveY[i]);

            ledGrid.setDataByte (triangleWaveOffset + i, triangleWaveY[i]);
            int triangleByte = ledGrid.getDataByte (triangleWaveOffset + i);
            jassert (triangleByte == triangleWaveY[i]);
        }
    }

    void setWaveshapeType (uint8 type)
    {
        ledGrid.setDataByte (0, type);
    }

    uint32 getHeapSize() override
    {
        return totalDataSize;
    }

    String getLittleFootProgram() override
    {
        return R"littlefoot(

        int yOffset;

        int min (int a, int b)
        {
            if (a > b)
                return b;

            return a;
        }

        int max (int a, int b)
        {
            if (a > b)
                return a;

            return b;
        }

        void drawLEDCircle (int x0, int y0)
        {
            setLED (x0, y0, 0xffff0000);

            int minLedIndex = 0;
            int maxLedIndex = 14;

            setLED (min (x0 + 1, maxLedIndex), y0, 0xff660000);
            setLED (max (x0 - 1, minLedIndex), y0, 0xff660000);
            setLED (x0, min (y0 + 1, maxLedIndex), 0xff660000);
            setLED (x0, max (y0 - 1, minLedIndex), 0xff660000);

            setLED (min (x0 + 1, maxLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000);
            setLED (min (x0 + 1, maxLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000);
            setLED (max (x0 - 1, minLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000);
            setLED (max (x0 - 1, minLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000);
        }

        void repaint()
        {
            // Clear LEDs to black
            fillRect (0xff000000, 0, 0, 15, 15);

            // Get the waveshape type
            int type = getHeapByte (0);
            int offset = 1 + (type * 45) + yOffset;

            for (int x = 0; x < 15; ++x)
            {
                int y = getHeapByte (offset + x);

                // Draw a vertical line if flag is set or draw an LED circle
                if (y == 20)
                {
                    for (int i = 0; i < 15; ++i)
                        drawLEDCircle (x, i);
                }
                else if (x % 2 == 0)
                {
                    drawLEDCircle (x, y);
                }
            }

            if (++yOffset == 30)
                yOffset = 0;

        }

        )littlefoot";
    }

private:
    static constexpr uint32 waveshapeType      = 0;   // 1 byte
    static constexpr uint32 sineWaveOffset     = 1;   // 1 byte * 45
    static constexpr uint32 squareWaveOffset   = 46;  // 1 byte * 45
    static constexpr uint32 sawWaveOffset      = 91;  // 1 byte * 45
    static constexpr uint32 triangleWaveOffset = 136; // 1 byte * 45

    static constexpr uint32 totalDataSize = triangleWaveOffset + 45;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveshapeProgram)
};

#endif  // WAVESHAPEPROGRAM_H_INCLUDED
