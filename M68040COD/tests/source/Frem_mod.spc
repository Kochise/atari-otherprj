verbose
#smod_oper dst : norm, src : zero
fmod.d [s]3f800000 0000_0000_0000_0000
fmod.d [s]bf800000 0000_0000_0000_0000
fmod.d [s]3f800000 8000_0000_0000_0000
fmod.d [s]bf800000 8000_0000_0000_0000
#smod_fpn dst : norm, src : inf
fmod.d [s]3f800000 7ff0_0000_0000_0000
fmod.d [s]bf800000 7ff0_0000_0000_0000
fmod.d [s]3f800000 fff0_0000_0000_0000
fmod.d [s]bf800000 fff0_0000_0000_0000
#smod_snan dst : norm, src : qnan
fmod.d [s]3f800000 7fff_0000_0000_0000
fmod.d [s]bf800000 7fff_0000_0000_0000
fmod.d [s]3f800000 ffff_0000_0000_0000
fmod.d [s]bf800000 ffff_0000_0000_0000
#smod_snan dst : norm, src : snan
fmod.d [s]3f800000 7ff7_0000_0000_0000
fmod.d [s]bf800000 7ff7_0000_0000_0000
fmod.d [s]3f800000 fff7_0000_0000_0000
fmod.d [s]bf800000 fff7_0000_0000_0000
#smod_zro dst : zero, src : norm
fmod.d [s]00000000 3ff0_0000_0000_0000
fmod.d [s]80000000 3ff0_0000_0000_0000
fmod.d [s]00000000 bff0_0000_0000_0000
fmod.d [s]80000000 bff0_0000_0000_0000
#smod_oper dst : zero, src : zero
fmod.d [s]00000000 0000_0000_0000_0000
fmod.d [s]80000000 0000_0000_0000_0000
fmod.d [s]00000000 8000_0000_0000_0000
fmod.d [s]80000000 8000_0000_0000_0000
#smod_zro dst : zero, src : inf
fmod.d [s]00000000 7ff0_0000_0000_0000
fmod.d [s]80000000 7ff0_0000_0000_0000
fmod.d [s]00000000 fff0_0000_0000_0000
fmod.d [s]80000000 fff0_0000_0000_0000
#smod_snan dst : zero, src : qnan
fmod.d [s]00000000 7fff_0000_0000_0000
fmod.d [s]80000000 7fff_0000_0000_0000
fmod.d [s]00000000 ffff_0000_0000_0000
fmod.d [s]80000000 ffff_0000_0000_0000
#smod_snan dst : zero, src : snan
fmod.d [s]00000000 7ff7_0000_0000_0000
fmod.d [s]80000000 7ff7_0000_0000_0000
fmod.d [s]00000000 fff7_0000_0000_0000
fmod.d [s]80000000 fff7_0000_0000_0000
#smod_oper dst : inf, src : norm
fmod.d [s]7f800000 3ff0_0000_0000_0000
fmod.d [s]ff800000 3ff0_0000_0000_0000
fmod.d [s]7f800000 bff0_0000_0000_0000
fmod.d [s]ff800000 bff0_0000_0000_0000
#smod_oper dst : inf, src : zero
fmod.d [s]7f800000 0000_0000_0000_0000
fmod.d [s]ff800000 0000_0000_0000_0000
fmod.d [s]7f800000 8000_0000_0000_0000
fmod.d [s]ff800000 8000_0000_0000_0000
#smod_oper dst : inf, src : inf
fmod.d [s]7f800000 7ff0_0000_0000_0000
fmod.d [s]ff800000 7ff0_0000_0000_0000
fmod.d [s]7f800000 fff0_0000_0000_0000
fmod.d [s]ff800000 fff0_0000_0000_0000
#smod_snan dst : inf, src : qnan
fmod.d [s]7f800000 7fff_0000_0000_0000
fmod.d [s]ff800000 7fff_0000_0000_0000
fmod.d [s]7f800000 ffff_0000_0000_0000
fmod.d [s]ff800000 ffff_0000_0000_0000
#smod_snan dst : inf, src : snan
fmod.d [s]7f800000 7ff7_0000_0000_0000
fmod.d [s]ff800000 7ff7_0000_0000_0000
fmod.d [s]7f800000 fff7_0000_0000_0000
fmod.d [s]ff800000 fff7_0000_0000_0000
#smdd_dnan dst : qnan, src : norm
fmod.d [s]7ff00000 3ff0_0000_0000_0000
fmod.d [s]fff00000 3ff0_0000_0000_0000
fmod.d [s]7ff00000 bff0_0000_0000_0000
fmod.d [s]fff00000 bff0_0000_0000_0000
#smod_dnan dst : snan, src : norm
fmod.d [s]7fb00000 3ff0_0000_0000_0000
fmod.d [s]ffb00000 3ff0_0000_0000_0000
fmod.d [s]7fb00000 bff0_0000_0000_0000
fmod.d [s]ffb00000 bff0_0000_0000_0000
#smod_dnan dst : qnan, src : zero
fmod.d [s]7ff00000 0000_0000_0000_0000
fmod.d [s]fff00000 0000_0000_0000_0000
fmod.d [s]7ff00000 8000_0000_0000_0000
fmod.d [s]fff00000 8000_0000_0000_0000
#smod_dnan dst : snan, src : zero
fmod.d [s]7fb00000 0000_0000_0000_0000
fmod.d [s]ffb00000 0000_0000_0000_0000
fmod.d [s]7fb00000 8000_0000_0000_0000
fmod.d [s]ffb00000 8000_0000_0000_0000
#smod_dnan dst : qnan, src : inf
fmod.d [s]7ff00000 7ff0_0000_0000_0000
fmod.d [s]fff00000 7ff0_0000_0000_0000
fmod.d [s]7ff00000 fff0_0000_0000_0000
fmod.d [s]fff00000 fff0_0000_0000_0000
#smod_dnan dst : snan, src : inf
fmod.d [s]7fb00000 7ff0_0000_0000_0000
fmod.d [s]ffb00000 7ff0_0000_0000_0000
fmod.d [s]7fb00000 fff0_0000_0000_0000
fmod.d [s]ffb00000 fff0_0000_0000_0000
#smod_dnan dst : qnan, src : qnan
fmod.d [s]7ff00000 7fff_0000_0000_0000
fmod.d [s]fff00000 7fff_0000_0000_0000
fmod.d [s]7ff00000 ffff_0000_0000_0000
fmod.d [s]fff00000 ffff_0000_0000_0000
#smod_dnan dst : qnan, src : snan
fmod.d [s]7ff00000 7ffb_0000_0000_0000
fmod.d [s]fff00000 7ffb_0000_0000_0000
fmod.d [s]7ff00000 fffb_0000_0000_0000
fmod.d [s]fff00000 fffb_0000_0000_0000
#smod_dnan dst : snan, src : qnan
fmod.d [s]7fb00000 7fff_0000_0000_0000
fmod.d [s]ffb00000 7fff_0000_0000_0000
fmod.d [s]7fb00000 ffff_0000_0000_0000
fmod.d [s]ffb00000 ffff_0000_0000_0000
#smod_dnan dst : snan, src : snan
fmod.d [s]7fb00000 7ffb_0000_0000_0000
fmod.d [s]ffb00000 7ffb_0000_0000_0000
fmod.d [s]7fb00000 fffb_0000_0000_0000
fmod.d [s]ffb00000 fffb_0000_0000_0000
#srem_oper dst : norm, src : zero
frem.d [s]3f800000 0000_0000_0000_0000
frem.d [s]bf800000 0000_0000_0000_0000
frem.d [s]3f800000 8000_0000_0000_0000
frem.d [s]bf800000 8000_0000_0000_0000
#srem_fpn dst : norm, src : inf
frem.d [s]3f800000 7ff0_0000_0000_0000
frem.d [s]bf800000 7ff0_0000_0000_0000
frem.d [s]3f800000 fff0_0000_0000_0000
frem.d [s]bf800000 fff0_0000_0000_0000
#srem_snan dst : norm, src : qnan
frem.d [s]3f800000 7fff_0000_0000_0000
frem.d [s]bf800000 7fff_0000_0000_0000
frem.d [s]3f800000 ffff_0000_0000_0000
frem.d [s]bf800000 ffff_0000_0000_0000
#srem_snan dst : norm, src : snan
frem.d [s]3f800000 7ff7_0000_0000_0000
frem.d [s]bf800000 7ff7_0000_0000_0000
frem.d [s]3f800000 fff7_0000_0000_0000
frem.d [s]bf800000 fff7_0000_0000_0000
#srem_zro dst : zero, src : norm
frem.d [s]00000000 3ff0_0000_0000_0000
frem.d [s]80000000 3ff0_0000_0000_0000
frem.d [s]00000000 bff0_0000_0000_0000
frem.d [s]80000000 bff0_0000_0000_0000
#srem_oper dst : zero, src : zero
frem.d [s]00000000 0000_0000_0000_0000
frem.d [s]80000000 0000_0000_0000_0000
frem.d [s]00000000 8000_0000_0000_0000
frem.d [s]80000000 8000_0000_0000_0000
#srem_zro dst : zero, src : inf
frem.d [s]00000000 7ff0_0000_0000_0000
frem.d [s]80000000 7ff0_0000_0000_0000
frem.d [s]00000000 fff0_0000_0000_0000
frem.d [s]80000000 fff0_0000_0000_0000
#srem_snan dst : zero, src : qnan
frem.d [s]00000000 7fff_0000_0000_0000
frem.d [s]80000000 7fff_0000_0000_0000
frem.d [s]00000000 ffff_0000_0000_0000
frem.d [s]80000000 ffff_0000_0000_0000
#srem_snan dst : zero, src : snan
frem.d [s]00000000 7ff7_0000_0000_0000
frem.d [s]80000000 7ff7_0000_0000_0000
frem.d [s]00000000 fff7_0000_0000_0000
frem.d [s]80000000 fff7_0000_0000_0000
#srem_oper dst : inf, src : norm
frem.d [s]7f800000 3ff0_0000_0000_0000
frem.d [s]ff800000 3ff0_0000_0000_0000
frem.d [s]7f800000 bff0_0000_0000_0000
frem.d [s]ff800000 bff0_0000_0000_0000
#srem_oper dst : inf, src : zero
frem.d [s]7f800000 0000_0000_0000_0000
frem.d [s]ff800000 0000_0000_0000_0000
frem.d [s]7f800000 8000_0000_0000_0000
frem.d [s]ff800000 8000_0000_0000_0000
#srem_oper dst : inf, src : inf
frem.d [s]7f800000 7ff0_0000_0000_0000
frem.d [s]ff800000 7ff0_0000_0000_0000
frem.d [s]7f800000 fff0_0000_0000_0000
frem.d [s]ff800000 fff0_0000_0000_0000
#srem_snan dst : inf, src : qnan
frem.d [s]7f800000 7fff_0000_0000_0000
frem.d [s]ff800000 7fff_0000_0000_0000
frem.d [s]7f800000 ffff_0000_0000_0000
frem.d [s]ff800000 ffff_0000_0000_0000
#srem_snan dst : inf, src : snan
frem.d [s]7f800000 7ff7_0000_0000_0000
frem.d [s]ff800000 7ff7_0000_0000_0000
frem.d [s]7f800000 fff7_0000_0000_0000
frem.d [s]ff800000 fff7_0000_0000_0000
#smdd_dnan dst : qnan, src : norm
frem.d [s]7ff00000 3ff0_0000_0000_0000
frem.d [s]fff00000 3ff0_0000_0000_0000
frem.d [s]7ff00000 bff0_0000_0000_0000
frem.d [s]fff00000 bff0_0000_0000_0000
#srem_dnan dst : snan, src : norm
frem.d [s]7fb00000 3ff0_0000_0000_0000
frem.d [s]ffb00000 3ff0_0000_0000_0000
frem.d [s]7fb00000 bff0_0000_0000_0000
frem.d [s]ffb00000 bff0_0000_0000_0000
#srem_dnan dst : qnan, src : zero
frem.d [s]7ff00000 0000_0000_0000_0000
frem.d [s]7ff00000 0000_0000_0000_0000
frem.d [s]fff00000 8000_0000_0000_0000
frem.d [s]fff00000 8000_0000_0000_0000
#srem_dnan dst : snan, src : zero
frem.d [s]7fb00000 0000_0000_0000_0000
frem.d [s]ffb00000 0000_0000_0000_0000
frem.d [s]7fb00000 8000_0000_0000_0000
frem.d [s]ffb00000 8000_0000_0000_0000
#srem_dnan dst : qnan, src : inf
frem.d [s]7ff00000 7ff0_0000_0000_0000
frem.d [s]fff00000 7ff0_0000_0000_0000
frem.d [s]7ff00000 fff0_0000_0000_0000
frem.d [s]fff00000 fff0_0000_0000_0000
#srem_dnan dst : snan, src : inf
frem.d [s]7fb00000 7ff0_0000_0000_0000
frem.d [s]ffb00000 7ff0_0000_0000_0000
frem.d [s]7fb00000 fff0_0000_0000_0000
frem.d [s]ffb00000 fff0_0000_0000_0000
#srem_dnan dst : qnan, src : qnan
frem.d [s]7ff00000 7fff_0000_0000_0000
frem.d [s]fff00000 7fff_0000_0000_0000
frem.d [s]7ff00000 ffff_0000_0000_0000
frem.d [s]fff00000 ffff_0000_0000_0000
#srem_dnan dst : qnan, src : snan
frem.d [s]7ff00000 7ffb_0000_0000_0000
frem.d [s]fff00000 7ffb_0000_0000_0000
frem.d [s]7ff00000 fffb_0000_0000_0000
frem.d [s]fff00000 fffb_0000_0000_0000
#srem_dnan dst : snan, src : qnan
frem.d [s]7fb00000 7fff_0000_0000_0000
frem.d [s]ffb00000 7fff_0000_0000_0000
frem.d [s]7fb00000 ffff_0000_0000_0000
frem.d [s]ffb00000 ffff_0000_0000_0000
#srem_dnan dst : snan, src : snan
frem.d [s]7fb00000 7ffb_0000_0000_0000
frem.d [s]ffb00000 7ffb_0000_0000_0000
frem.d [s]7fb00000 fffb_0000_0000_0000
frem.d [s]ffb00000 fffb_0000_0000_0000
