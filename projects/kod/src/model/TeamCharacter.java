/* $Id: TeamCharacter.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * TeamCharacter.java
 * 
 * Extension for character.
 */

package src.model;

import src.utils.Constants;

public abstract class TeamCharacter extends Character 
{
    private boolean isFrog;

    /**
     * Constructor.
     * 
     * @param gender
     */
    public TeamCharacter(char gender) 
    {
    	super(gender);
    	this.isFrog = false;
    }

    /**
     * Player kisses another character.
     * Returns true if the monster was kissed, false otherwise.
     * 
     * @param player The character who's being kissed
     * @return boolean
     */
    public boolean kiss(Character player) 
    {
        /* Check if kissed player is the monster. */
    	if (player instanceof Monster)
    		return true;

    	/* Not the monster. */
        TeamCharacter kissed = (TeamCharacter)player;  

        if (kissed.getPlayerType().getPlayerTypeChar() == this.getPlayerType().getPlayerTypeChar() && kissed.isFrog()) {
            /* Player kisses a frog of the same type. */
        	kissed.setFrog(false);

        } else if (kissed.getPlayerType() != this.getPlayerType() && !kissed.isFrog()) {
            /* Player kisses another type. */
        	if (findActiveAttribute(Constants.ATTR_LIPSTICK) == null)
        		this.setFrog(true);
        }

        return false;
    }

    /**
     * Sets the Frog to true or false.
     * 
     * @param value
     */
    public void setFrog(boolean value)
    {
        this.isFrog = value;
    }

    /**
     * Returns the Frog enabled status.
     *  
     * @return boolean
     */
    public boolean isFrog()
    {
        return isFrog;
    }

    /**
     * Returns true if we move our character.
     * 
     * @param cs
     * @param changeAttributes
     * @param moves
     * @return boolean
     */
	public boolean moveTo(CharacterSpot cs, boolean changeAttributes, int moves)
	{
		/* We can't move if we are a frog. */
		if (isFrog())
			return false;

		return super.moveTo(cs, changeAttributes, moves);
	}

    /*
     * To be implemented by subclasses.
     */
    public abstract Character getPlayerType();
}
