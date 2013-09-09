#!/usr/bin/wish
wm title . "Koules 1.4"
# This is simple user (anti)friendly dialog for starting koules
# Only reason why I did it is that I wanted to try tcl/tk
#files - configure here if paths are differ
set xkoules "/usr/bin/X11/xkoules"
set koulessvga "/usr/local/bin/koules.svga"
set koulessound "/usr/local/lib/koules/creator1.raw"
set config "~/.xkoules.opt"
set revision 1.1
# Set default
set GameMode Single
set Shm 1
set private 0
set Fullscreen 0
set ScreenSize "-l"
set Sound 1
set Monochrome 0
set blackwhite 0
set Cursor 0
set Level 1
set Host localhost
set Deatmatch 0
set SPort 12345
set CPort 12345
set Difficulty 2
set SScreenSize -l
if {[file exists $config] == 1} {
  set FileId [open $config "r"]
  if {[gets $FileId] == $revision } {
  set GameMode [gets $FileId]
  set ScreenSize [gets $FileId ]
  set private [gets $FileId ]
  set blackwhite [gets $FileId ]
  set Cursor [gets $FileId ]
  set Shm [gets $FileId ]
  set Monochrome [gets $FileId ]
  set Sound [gets $FileId ]
  set Fullscreen [gets $FileId ]
  set CPort [gets $FileId ]
  set Host [gets $FileId ]
  set SPort [gets $FileId ]
  set Level [gets $FileId ]
  set Difficulty [gets $FileId ]
  set Deathmatch [gets $FileId ]
  set SScreenSize [gets $FileId ]
  } else {puts "Ignoring config file...another version"}
  close $FileId
}

frame .gamemode -relief raised -borderwidth 3
radiobutton .gamemode.single \
   -value "Single"\
   -variable GameMode\
   -text "Single"\
   -command "SetMode"
radiobutton .gamemode.client \
   -value "Client"\
   -variable GameMode\
   -command "SetMode" \
   -text "Client"
radiobutton .gamemode.server \
   -value "Server"\
   -variable GameMode\
   -command "SetMode"\
   -text "Server"

pack .gamemode.single\
     .gamemode.client\
     .gamemode.server\
   -side left -expand yes
frame .buttons -relief raised
frame .single -relief raised
frame .single.size -relief raised
frame .single.gameopt -relief raised
frame .server -relief raised
frame .server.left -relief raised
frame .server.left.size -relief raised
frame .server.left.data -relief raised
frame .server.text -relief raised
frame .server.difficulty -relief raised

radiobutton .server.difficulty.0 \
   -value "0"\
   -variable Difficulty\
   -text "Nightmare"
radiobutton .server.difficulty.1 \
   -value "1"\
   -variable Difficulty\
   -text "Hard"
radiobutton .server.difficulty.2 \
   -value "2"\
   -variable Difficulty\
   -text "Medium"
radiobutton .server.difficulty.3 \
   -value "3"\
   -variable Difficulty\
   -text "Easy"
radiobutton .server.difficulty.4 \
   -value "4"\
   -variable Difficulty\
   -text "Very easy"
radiobutton .server.left.size.s \
   -value "-l"\
   -variable SScreenSize\
   -text "350x200 and 640x480"
radiobutton .server.left.size.m \
   -value "-W"\
   -variable SScreenSize\
   -text "320x200 and 640x380"
radiobutton .server.left.size.l \
   -value "-E"\
   -variable SScreenSize\
   -text "900x600" 
checkbutton .server.left.size.deathmatch \
   -variable Deathmatch\
   -text "Deathmatch"\
   -command "SetDeath"
frame .server.left.data.level
frame .server.left.data.port
label .server.left.data.level.label -text Level
label .server.left.data.port.label -text Port
entry .server.left.data.level.entry -relief sunken -width 3 -textvariable Level
#.server.left.data.level.entry insert 0 $Level
entry .server.left.data.port.entry -relief sunken -width 6 -textvariable SPort
#.server.left.data.port.entry insert 0 $SPort 
frame .client
label .client.hostlabel -text Host
label .client.portlabel -text Port
entry .client.host -relief sunken -textvariable Host
entry .client.port -relief sunken -width 6 -textvariable CPort
#.client.host insert 0 $Host
#.client.port insert 0 $CPort
pack .client.hostlabel\
     .client.host\
     .client.portlabel\
     .client.port -side left -fill x
#server.gameopt.level insert $Level
pack .server.left.data.level.label\
     .server.left.data.level.entry -side left -fill x
pack .server.left.data.port.label\
     .server.left.data.port.entry -side left -fill x
pack .server.left.data.port\
     .server.left.data.level -side top -fill x
pack .server.difficulty.0\
     .server.difficulty.1\
     .server.difficulty.2\
     .server.difficulty.3\
     .server.difficulty.4\
     -padx 10 -expand 1 -anchor w
pack .server.left.size.s\
     .server.left.size.m\
     .server.left.size.l \
     .server.left.size.deathmatch -padx 10 -expand 1 -anchor w
pack .server.left.data\
     .server.left.size
#normal game options
radiobutton .single.size.s \
   -value "-s"\
   -variable ScreenSize\
   -text "320x250"
radiobutton .single.size.m \
   -value "-l"\
   -variable ScreenSize\
   -text "640x480"
radiobutton .single.size.l \
   -value "-E"\
   -variable ScreenSize\
   -text "900x600" 
checkbutton .single.size.mode \
   -variable Fullscreen\
   -text "Fullscreen"\
   -command "SetExtra"
checkbutton .single.gameopt.sound\
   -variable Sound\
   -text "sound"
checkbutton .single.gameopt.shm\
   -variable Shm\
   -text "Use Shared memory"
checkbutton .single.gameopt.monochrome\
   -variable Monochrome\
   -text "Monochrome"
checkbutton .single.gameopt.blackwhite\
   -variable blackwhite\
   -text "Black and white" 
checkbutton .single.gameopt.private\
   -variable private\
   -text "Private colormap"
checkbutton .single.gameopt.cursor\
   -variable Cursor\
   -text "Game own mouse cursor"
button .buttons.start -text Start -command "Start"
button .buttons.quit -text Quit -command exit
pack .buttons.start\
     .buttons.quit\
     -expand 1 -fill x -side left
pack .single.size.s\
     .single.size.m\
     .single.size.l\
     .single.size.mode -padx 10 -fill x -expand y -anchor w
pack .single.gameopt.sound \
     .single.gameopt.shm \
     .single.gameopt.private \
     .single.gameopt.monochrome \
     .single.gameopt.blackwhite \
     .single.gameopt.cursor  -side top  -anchor w
pack .single.size\
     .single.gameopt -side left
pack .server.left .server.difficulty -side left -fill x
pack .gamemode -side top -expand y -fill x -pady 2m
pack .single  -expand y -fill x -pady 2m
pack .buttons -side bottom -expand y -fill x -pady 2m
if {[file exists $xkoules] == 1} {
  set Koulesexe $xkoules
} else {
   if {[file exists $koulessvga] == 1} {
     set Fullscreen 1
     .single.size.mode configure -state disabled
   } else {
     puts "Koules executable not found"
     exit
   }
}
if {[file exists $koulessound] == 0} {
  set Sound 0
  .single.gameopt.sound configure -state disabled
}
if {[file exists $koulessvga] == 1} {
  set Koulesexe $koulessvga
} else {
  set Fullscreen 0
  .single.size.mode configure -state disabled
}
proc SetDeath {} {
  global Deathmatch
  if {$Deathmatch == "1" } {
    .server.left.data.level.entry configure -state disabled
  } else {
    .server.left.data.level.entry configure -state normal
  }
}
SetDeath
proc SetExtra {} {
  global Fullscreen
  global ScreenSize
  if {$Fullscreen == "1" } {
     .single.size.l configure -state disabled
     .single.size.s configure -text 320x200
     .single.gameopt.shm configure -state disabled
     .single.gameopt.private configure -state disabled
     .single.gameopt.monochrome configure -state disabled
     .single.gameopt.blackwhite configure -state disabled
     .single.gameopt.cursor configure -state disabled
     if {$ScreenSize == "-E"} {
       set ScreenSize "-l"
       #.single.size.m configure -state active
     }
  } else {
     .single.size.l configure -state normal
     .single.size.s configure -text 320x250
     .single.gameopt.shm configure -state normal
     .single.gameopt.private configure -state normal
     .single.gameopt.monochrome configure -state normal
     .single.gameopt.blackwhite configure -state normal
     .single.gameopt.cursor configure -state normal
  }
}
SetExtra
proc Start {} {
  global GameMode
  global ScreenSize
  global private
  global blackwhite
  global Cursor
  global Shm
  global Monochrome
  global Sound
  global Fullscreen
  global config
  global revision
  #client options
  global CPort
  global Host
  #server options
  global SPort
  global Level
  global Difficulty
  global Deathmatch
  global SScreenSize
  global xkoules
  global koulessvga
  global Koulesexe
  set FileId [open $config "w"]
  puts $FileId $revision
  puts $FileId $GameMode
  puts $FileId $ScreenSize
  puts $FileId $private
  puts $FileId $blackwhite
  puts $FileId $Cursor
  puts $FileId $Shm
  puts $FileId $Monochrome
  puts $FileId $Sound
  puts $FileId $Fullscreen
  puts $FileId $CPort
  puts $FileId $Host
  puts $FileId $SPort
  puts $FileId $Level
  puts $FileId $Difficulty
  puts $FileId $Deathmatch
  puts $FileId $SScreenSize
  close $FileId
  if {$GameMode != "Server" } {
    set opt $ScreenSize
    if {$Sound == "0"} {
      set opt "$opt -d"
    }
    if {$Fullscreen == "0"} {
    set exec $xkoules
    if {$private == "1"} {
      set opt "$opt -p"
    }
    if {$Monochrome == "1"} {
      set opt "$opt -m"
    }
    if {$blackwhite == "1"} {
      set opt "$opt -b"
    }
    if {$Cursor == "1"} {
      set opt "$opt -x"
    }
    if {$Shm == "0"} {
      set opt "$opt -M"
    }
    } else {
      set exec $koulessvga
    }
    if {$GameMode=="Client"} {
      set opt "$opt -C $Host -P $CPort"
    }
    #puts "$exec $opt"
    destroy .
    exec /bin/sh "-c" "$exec $opt"
  } else {
    set exec  $Koulesexe
    set opt "-S -P $SPort $SScreenSize -D $Difficulty"
    if {$Deathmatch == "1"} {
      set opt "$opt -K"
    }
    #puts "$exec $opt"
    destroy .
    exec xterm "-iconic" "-title" "Koules server" "-e" "/bin/sh" "-c" "$exec $opt"
  }
}
pack .gamemode\
     .buttons -expand 1 -side top -pady 2 -anchor w
proc SetMode {} {
  global GameMode
  if {$GameMode == "Single" } {
    pack forget .server
    pack forget .client
    pack .single\
         -before .buttons -expand 1 -side top -pady 2 -anchor w
  }
  if {$GameMode == "Server"} {
    pack forget .single
    pack forget .client
    pack .server -before .buttons -expand 1 -side top -pady 2 -anchor w
  }
  if {$GameMode == "Client"} {
    pack forget .server
    pack .single\
         -before .buttons -expand 1 -side top -pady 2 -anchor w
    pack .client\
         -before .buttons -expand 1 -side top -pady 2 -anchor w
  }
}
puts $GameMode
bind . <Return> Start
bind . <Escape> exit
bind . <Control-c> exit
SetMode
