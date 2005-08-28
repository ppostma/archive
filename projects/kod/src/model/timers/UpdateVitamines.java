/* $Id: UpdateVitamines.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * UpdateVitamines.java
 * 
 * Update the vitamines in the Playfield.
 */

package src.model.timers;

import src.model.Playfield;
import src.utils.TimerCallbackObject;

public class UpdateVitamines implements TimerCallbackObject
{
    public UpdateVitamines()
    {
    }
    
    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

        pf.updateVitamines();
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in UpdateVitamines timer: " + e.getMessage());
    }
}
