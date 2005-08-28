/* $Id: Spot.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Spot.java
 * 
 * General class for different spot types.
 */

package src.model;

public abstract class Spot 
{
	/**
	 * Constructor.
	 */
    public Spot() 
    {
    }

    /*
     * To be implemented by subclasses.
     */
    public abstract char getSpotChar();
}
