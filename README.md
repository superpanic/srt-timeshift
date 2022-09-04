# srt-timeshift
A very basic command line tool I needed for time-shifting .srt subbtitles.

Provide two arguments:  
PATH to ```subtitles.srt``` file  
TIME in milliseconds ```1000``` or ```-1000```

Example: ```./srt-timeshift ./gladiator.srt -69420```

For longer subs, adjust ```#define MAX_LINES 20000``` (Currently set to 20000.)
