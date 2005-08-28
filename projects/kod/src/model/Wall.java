/* $Id: Wall.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Wall.java
 * 
 * Definition for a wall in a room.
 */

package src.model;

import src.utils.Constants;

public class Wall extends Spot 
{
	/**
	 * Constructor.
	 */
    public Wall() 
    {
    }

    /**
     * Returns the defined char for wall.
     * 
     * @return char
     */
	public char getSpotChar()
	{
		return Constants.WALL;
	}
}
