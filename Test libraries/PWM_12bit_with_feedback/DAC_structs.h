struct DAC_chars
{
  unsigned char LSC;
  unsigned char MSC;
};

union DAC_union
{
  DAC_chars DAC_char_segs;
  unsigned int DAC_strct_raw;
};

  DAC_union ch4A;
  DAC_union ch4D;
