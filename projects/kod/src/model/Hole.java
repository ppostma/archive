/* $Id: Hole.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Hole.java
 * 
 * Hole spot.
 */

package src.model;

import src.utils.Constants;

public class Hole extends CharacterSpot 
{
    /**
     * Constructor.
     */
    public Hole() 
    {
    }

	/**
	 * Returns the defined char for hole.
	 * 
	 * @return char
	 */
	public char getSpotChar()
	{
		return Constants.HOLE;
	}
}
