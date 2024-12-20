# About


Our general design process started with the finalization of creating this physical implementation of 'Keep Talking and Nobody Explodes'. Once we decided this, we started coming up with our modules.

In the original game there are 2 main roles, the defuser and the instructor. We decided to follow this model for our own implementation as well. There are also a random assortment of 5 modules in the original, and we wanted to be able to showcase at least 5 for our design as well. 

We did some brainstorming, and settled on these:
- Main Module Timer
- Button
- Memory
- Keypad 
- Wires
- Morse Code

The biggest factors for us when deciding were:
- How feasible is it to make this module?
- Did we have fun playing the original game's version of this module?
- Do we think that this module will fit the vibe of the others in the box? 

Once we had all of our ideas modeled, we had to organize our layout. Specifically we spent a lot of time drawing out a Data Energy Flow Diagram, which shows the relationships between power, data, and human interaction. 

## Data Energy Flow Diagram
<img src="assets/DtS_Energy_Flow_Diagram.png" alt="Data Energy Flow Diagram" width="1000px">

A few notes to make this diagram a little clearer:
- There are only 2 users of the game, so all the users with scissors (defuser) are the same person.
- The pink 'Human Perception' lines are meant to reprisent a flow of data that is aquired or transmitted through human perception. This differs from our normal green 'Data' lines because there are no mistakes in transmitting that data, whereas with a human perception line, mistakes can (and will) be made, which is an intentional design decision of our game.
- The small seperated piece of the diagram in the bottom left is the system between the two users. The instructor gets information from the manual, then transmits that to the defuser, while the defuser is transmitting data regarding the game state to the instructor. 


From here, we can start getting in depth on each component of the game. By hovering over the 'about' page on the sidebar, you will see a dropdown guiding you to the Mechanical, Electrical, and Software pages