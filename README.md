<h2>What is it?</h2>
This is a trainer I wrote to learn the very basics of reverse engineering and memory editing in C++.

<h2>How do I use it?</h2>
Open NetHack, open the trainer, and use the commands. It is strongly recommended that you review the source and build the application yourself because of the sketchy implications of memory editing.

<h3>Commands List</h3>

```
STATS
=================
armorclass (num)                 set the armorclass to specified num.
strength (num)                   set the strength value to specified num.
intelligence (num)               set the intelligence value to specified num.
wisdom (num)                     set the wisdom value to specified num.
dexterity (num)                  set the dexterity value to specified num.
constitution (num)               set the constitution value to specified num.
charisma (num)                   set the charisma value to specified num.
health (num)/(max_num)           set the health and maximum health to specified nums.
power (num)/(max_num)            set the health and maximum health to specified nums.

NON-VISIBLE STATS
=================
viewinvis                        view the values of invisible stats.
nutrition (num)                  set the nutrition value to specified num.
exp (num)                        set the experience value to specified num.
pos (x, y)                       set the position to provided coordinates.

MISC
=================
wizard                           enter debug mode. (https://nethackwiki.com/wiki/Wizard_mode)
turn (num)                       set the turn value to specified num.
!(CMD)                           execute system command.
help                             view this menu.
quit                             exit the program.
```
