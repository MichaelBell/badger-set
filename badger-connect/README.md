# Badger Connect 4

Connect 4 for the Badger 2040, designed to work from a battery.  A coin cell should last for several hours of continuous play.

The pieces are squares and crosses, get 4 of your shape in a line to win.  The current player's turn is indicated by the piece that's drawn to the right of the board.

## Controls

* A: Move to left
* B: Drop piece in selected column
* C: Move to right

* UP: Refresh the screen - useful if the badger/duck is looking a bit tired
* DOWN: Reset the game and power off

## Power

The LED indicates whether power is on - while the screen is drawing and you can't do anything it is dim, when you can take a move it is bright.

Power is switched off when someone wins, or after about 20 seconds of no buttons being pressed.

If power is off, switch on by pressing any button.  If you press DOWN then a new game is started, any other button continues the existing game.  
Note that the active column resets to the centre.
