/* $Id: UpdateAttributes.java,v 1.1 2005-08-28 15:05:06 peter Exp $ */

/**
 * Kiss of Death
 * UpdateAttributes.java
 * 
 * Update the attributes in the playfield.
 */

package src.model.timers;

import src.model.Playfield;
import src.utils.TimerCallbackObject;

public class UpdateAttributes implements TimerCallbackObject
{
    public UpdateAttributes()
    {
    }

    public void ready()
    {
    	Playfield pf = Playfield.getInstance();

        pf.updateAttributes();
    }

    public void error(Exception e)
    {
    	System.err.println("Problem in UpdateAttributes timer: " + e.getMessage());
    }
}
