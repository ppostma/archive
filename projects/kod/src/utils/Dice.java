/* $Id: Dice.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Dice.java
 * 
 * Throws a dice.
 */

package src.utils;

public class Dice
{
    /**
     * Function for throwing a dice. The function returns a boolean and
     * the higher the chance is, the more chance to get a 'true'.
     * 
     * @param chance A value between 1 and 100
     * @return boolean
     */
	public static boolean doThrow(int chance)
	{
		return chance > Math.round(Math.random() * 100);
	}

	/**
	 * Throw a number between 0 and maxNumber.
	 * 
	 * @param maxNumber
	 * @return int
	 */
	public static int doThrowNumber(int maxNumber)
	{
		return (int)Math.round(Math.random() * maxNumber);
	}
}
