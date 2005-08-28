/* $Id: ViewCharacter.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ViewCharacter.java
 *
 * Character class for the view.
 * (this is NOT equal to the Character class in the model!!)
 */

package src.view;

import java.awt.Point;

import src.utils.Constants;

public class ViewCharacter
{
	public int health;
	public int energy;
	public int strength;
	public int speed;
	public int intelligence;
	public int armor;

	public char playerType;
	public char gender;
	public boolean dead;
	public boolean frog;
	public boolean kissing;
	public boolean defending;
	public boolean attacking;
	public boolean invisible;
	public boolean nightvision;
	public int roomID;
	public Point pos;

	public ViewUseAttribute attribute[];

	/**
	 * Constructor. This requires the playType and gender.
	 *
	 * @param playerType
	 * @param gender
	 */
	public ViewCharacter(char playerType, char gender)
	{
		pos = new Point(1, 1);
		attribute = new ViewUseAttribute[Constants.MAX_ATTRIBUTES_CARRIED];

		this.playerType = playerType;
		this.gender = gender;
		this.dead = false;
		this.frog = false;
		this.kissing = false;
		this.defending = false;
		this.attacking = false;
		this.invisible = false;
		this.nightvision = false;
		this.roomID = 0;

		this.health = 0;
		this.energy = 0;
		this.strength = 0;
		this.speed = 0;
		this.intelligence = 0;
		this.armor = 0;
	}
}
