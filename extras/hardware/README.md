Here are the KiCAD and Gerber files for creating the circuit boards this firmware is designed to run on.

You could theoretically run it on a bare ATmega328 chip or another board, but the code is specialized for this board and uses hard-coded pin numbers.

### Bill Of Materials

* 1x assembled PCB
   * buy assembled PCBs from PCBWay with all the surface mount soldering done for you
      * https://www.pcbway.com/project/shareproject/ByteSizedEncoderDecoder_v1_1_a5a4bd50.html
   * or do your own surface mount soldering:
      * circuit board:
         * If you buy PCBs from PCBWay through this link PCBWay generously gives me a 10% commission without costing you anything
            * https://www.pcbway.com/project/shareproject/Byte_Sized_Encoder_Decoder_v1_0_5b76fc5d.html
            * If you buy PCBs with another method (send ./PCB Production/gerbers-ByteSizedEncoderDecoder-v1.1.zip file) here's some info you might need: min track/spacing=6/6mil, min hole size=0.3mm
      * 2x JST SH https://www.digikey.com/en/products/detail/jst-sales-america-inc/SM04B-SRSS-TB/926710
      * 1x ATmega328PB TQFP https://www.digikey.com/en/products/detail/microchip-technology/ATMEGA328PB-AU/5638812
* 4x 1x08 0.1" female header pins
