/* $Id: ViewPlayer.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ViewPlayer.java
 *
 * Player class for the view.
 * (this is NOT equal to the Player class in the model!!)
 */

package src.view;

import src.utils.Constants;

public class ViewPlayer
{
	public String name;
	public char playerType;
	public boolean monster;
	public ViewCharacter[] character = new ViewCharacter[Constants.MAX_CHARACTERS];

	/**
	 * Constructor.
	 * 
	 * @param name
	 * @param playerType
	 * @param gender
	 */
	public ViewPlayer(String name, char playerType, char gender)
	{
		this.name = name;
		this.playerType= playerType;
		this.monster = false;
		this.character[0] = new ViewCharacter(playerType, gender);
	}
}