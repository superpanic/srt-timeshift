# srt-timeshift
Command line tool for time-shifting .srt subbtitles.

Provide two arguments:  
PATH to ```subtitles.srt``` file  
TIME in milliseconds ```1000``` or ```-1000```

Example: ```./srt-timeshift ./gladiator.srt -69420```

For longer subs, adjust MAX_LINES define. Currently set to 20000.
