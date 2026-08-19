// W65C02 8-bitarsdator — dispatcher
// Välj steg via platformio.ini build_flags: env:step1 … env:step11
// Default (inget definierat) = steg 9

#if defined(STEP1)
  #include "step1.inc"
#elif defined(STEP2)
  #include "step2.inc"
#elif defined(STEP3)
  #include "step3.inc"
#elif defined(STEP4)
  #include "step4.inc"
#elif defined(STEP5)
  #include "step5.inc"
#elif defined(STEP6)
  #include "step6.inc"
#elif defined(STEP7)
  #include "step7.inc"
#elif defined(STEP8)
  #include "step8.inc"
#elif defined(STEP9)
  #include "step9.inc"
#elif defined(STEP10)
  #include "step10.inc"
#elif defined(STEP11)
  #include "step11.inc"
#else
  #include "step9.inc"
#endif
