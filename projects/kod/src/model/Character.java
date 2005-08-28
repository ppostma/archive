/* $Id: Character.java,v 1.1 2005-08-28 15:05:07 peter Exp $ */

/**
 * Kiss of Death
 * Character.java
 * 
 * Main model class for character.
 */

package src.model;

import src.model.timers.EnergyRegeneration;
import src.model.timers.MoveDelay;
import src.utils.Constants;
import src.utils.Dice;
import src.utils.Timer;

public abstract class Character
{
    protected int health;
    protected int armor;
    protected int energy;
    protected int strength;
    protected int speed;
    protected int intelligence;
    protected char gender;
    protected Attribute attribute[];
    protected CharacterSpot location;
    private CharacterSpot defendingCharacterSpot;
    private Timer moveTimer;
    private Timer energyTimer;

    protected boolean dead;
    private boolean canMove;
    private boolean kissing;
    private boolean attacking;
    private boolean defending;
	private boolean invisible;
	private boolean nightvision;

    /**
     * Constructor.
     * 
     * @param gender
     */
    public Character(char gender)
    {
    	this.health = 100;
    	this.armor = 100;
    	this.energy = 100;
 	   	this.strength = 100;
 	   	this.speed = 100;
 	   	this.intelligence = 100;
 	   	this.dead = false;
 	   	this.defending = false;
 	   	this.attacking = false;
 	   	this.gender = gender;
 	   	this.canMove = true;
 	   	this.kissing = false;
 	   	this.invisible = false;
 	   	this.nightvision = false;

 	   	this.attribute = new Attribute[Constants.MAX_ATTRIBUTES_CARRIED];

 	   	energyTimer = new Timer(1000, 0, new EnergyRegeneration(this));
 	   	energyTimer.start();
    }

    /**
     * Destructor.
     */
    public void destroy()
    {
    	location.setCharacter(null);
    	location = null;

    	killTimers();
    }

    /**
     * Kills/destroys the timers.
     */
    public void killTimers()
    {
    	if (energyTimer != null)
    		energyTimer.stop();
    	if (moveTimer != null)
    		moveTimer.stop();

    	for (int i = 0; i < Constants.MAX_ATTRIBUTES_CARRIED; i++)
            if (attribute[i] != null)
                attribute[i].destroy();
    }

    /**
     * Get the location of the character.
     * 
     * @return CharacterSpot
     */
	public CharacterSpot getLocation()
	{
		return this.location;
	}

	/**
	 * Set the location of the character.
	 * 
	 * @param cs
	 */
	public void setLocation(CharacterSpot cs)
	{
		this.location = cs;
	}

	/**
	 * Set the character dead.
	 */
	public void setDead()
	{
		this.dead = true;
		if (this.location != null)
			this.location.setCharacter(null);
	}

	/**
	 * Set the character to alive.
	 */
	public void setAlive()
	{
		this.dead = false;
	}

	/**
	 * Check if the character is dead.
	 * 
	 * @return boolean
	 */
	public boolean isDead()
	{
		return this.dead;
	}

	/**
	 * Set the character defending or not-defending.
	 * 
	 * @param defending
	 */
	public void setDefending(boolean defending)
	{
		this.defending = defending;
	}

	/**
	 * Check if the character if defending.
	 * 
	 * @return boolean
	 */
	public boolean isDefending()
	{
		return this.defending;
	}

	/**
	 * Set the character attacking or not-attacking.
	 * 
	 * @param attacking
	 */
	public void setAttacking(boolean attacking)
	{
		this.attacking = attacking;
	}

	/**
	 * Check if the character is attacking.
	 * 
	 * @return boolean
	 */
	public boolean isAttacking()
	{
		return this.attacking;
	}

	/**
	 * Check if the character is attacking, defending or kissing.
	 * 
	 * @return boolean
	 */
	public boolean isAction()
	{
	    return this.attacking || this.defending || this.kissing;
	}

	/**
     * Get the health.
     * 
     * @return int
     */
	public int getHealth()
	{
		return health;
	}

	/**
	 * Set the health.
	 * 
	 * @param h
	 */
	public void setHealth(int h)
	{
		health = h;

		if (health <= 0) {
		    health = 0;
		    this.setDead();
		} else if (health > 100)
		    health = 100;
	}

	/**
	 * Get the energy.
	 *  
	 * @return int
	 */
	public int getEnergy()
	{
		return energy;
	}

	/**
	 * Set the energy.
	 * 
	 * @param e
	 */
	public void setEnergy(int e)
	{
		energy = e;

		if (energy < 0)
		    energy = 0;
		else if (energy > 100)
		    energy = 100;
	}

	/**
     * Get the strength.
     * 
     * @return int
     */
	public int getStrength()
	{
		return strength;
	}

	/**
	 * Set the strength.
	 * 
	 * @param s
	 */
	public void setStrength(int s)
	{
		strength = s;

		if (strength < 0)
		    strength = 0;
		else if (strength > 100)
		    strength = 100;
	}

	/**
	 * Get the speed.
	 * 
	 * @return int
	 */
	public int getSpeed()
	{
		return speed;
	}

	/**
	 * Set the speed.
	 * 
	 * @param s
	 */
	public void setSpeed(int s)
	{
		speed = s;

		if (speed < 0)
		    speed = 0;
		else if (speed > 100)
		    speed = 100;
	}

	/**
	 * Get the intelligence.
	 *  
	 * @return int
	 */
	public int getIntelligence()
	{
		return intelligence;
	}

	/**
	 * Set the intelligence.
	 * 
	 * @param i
	 */
	public void setIntelligence(int i)
	{
		intelligence = i;

		if (intelligence < 0)
		    intelligence = 0;
		else if (intelligence > 100)
		    intelligence = 100;
	}

	/**
	 * Get the armor.
	 *  
	 * @return int
	 */
	public int getArmor()
	{
		return armor;
	}

	/**
	 * Set the armor.
	 * 
	 * @param a
	 */
	public void setArmor(int a)
	{
		armor = a;

		if (armor < 0)
		    armor = 0;
		else if (armor > 100)
		    armor = 100;
	}

	/**
	 * Get the gender of the character.
	 * 
	 * @return char
	 */
    public char getGender() 
    {
        return this.gender; 
    }

    /**
	 * Set the gender of the character.
	 * 
	 * @param g
	 */
	public void setGender(char g) 
	{
		this.gender = g; 
	}

    /**
	 * Return invisibility.
	 * 
     * @return boolean
	 */
	public boolean getInvisible()
	{
		return this.invisible; 
	}
	
    /**
	 * Return nightvision.
	 * 
     * @return boolean
	 */
	public boolean getNightvision() 
	{
		return this.nightvision; 
	}

    /**
	 * Enable or disable invisibility
	 * 
	 * @param b
	 */
	public void setInvisible(boolean b) 
	{
		this.invisible = b; 
	}
	
    /**
	 * Enable or disable nightvision
	 * 
	 * @param b
	 */
	public void setNightvision(boolean b) 
	{
		this.nightvision = b; 
	}
	
	/**
	 * Enable or disable movement of the character.
	 *  
	 * @param bMove
	 */
	public void canMove(boolean bMove)
	{
		if (moveTimer == null)
			this.canMove = bMove; 
	}

	/**
	 * Sets canMove to true. Only allowed to be called by the MoveDelay timer.
	 *
	 */
	public void timerSetMovable()
	{
		this.canMove = true;
	}

	/**
	 * Enable/disable kissing.
	 * 
	 * @param b
	 */
	public void setKissing(boolean b)
	{
	    this.kissing = b; 
	}
	
	/**
	 * Attacks a character.
	 * 
	 * @param attacked
	 * @return Character
	 */
    public Character attack(Character attacked) 
    {
    	if (!this.isDead()) {
    		if (attacked == null || attacked.isDead() == true) {
    			this.setEnergy(this.getEnergy() - 10); // if empty spot is attacked
    			return null;
    		}
    		return attacked.attacked(this);
    	}

    	return null;
    }

    /**
     * Character is being attacked.
     * 
     * @param attacker
     * @return Character, outcome of battle
     */
    public Character attacked(Character attacker) 
    {
		int attackedDamage;
		int damageChance;

		if (attacker.getEnergy() < Constants.MIN_ATTACK_ENERGY)
			return null;

		attackedDamage = ((attacker.strength / this.strength) * 30) + 15 - (this.armor / 20);
		if (this.defending && defendingCharacterSpot == attacker.location) {
			attackedDamage = attackedDamage / 2;
			damageChance = ((attacker.intelligence / this.intelligence) * 33);
		} else {
			damageChance = ((attacker.intelligence / this.intelligence) * 33) + 40;		
		}

		if (Dice.doThrow(damageChance))
			this.setHealth(this.getHealth() - attackedDamage);
		else
		    attacker.setEnergy( attacker.getEnergy() - 30);

		//if (this.getHealth() <= 0)
		//	return this;

		return this;
    }

    /**
     * Try defending attack.
     * 
     * @param cs
     */
    public void defend(CharacterSpot cs) 
    {   
    	if (!this.isDead())
    		defendingCharacterSpot = cs;
    }

    /**
	 * Pick up the attribute from the current floor spot.
	 * Returns the picked up attribute ID or -1 if the location isn't a
	 * floor spot or if there's no attribute on the floor spot or if we've
	 * reached maximum carried attributes.
	 * 
	 * @return int
	 */
    public int pickUp() 
    {
    	if (!this.isDead()) {
    		int index = -1;

    		/* Only pick up from Floor spots. */
    		if (location instanceof Floor) {

    			/* Check if there's room in the backpack. */
    			for (int i = 0; i < Constants.MAX_ATTRIBUTES_CARRIED; i++) {
    				if (attribute[i] == null) {
    					index = i;
    					break;
    				}
    			}

    			if (index != -1) {
    				/* Pick up the attribute from the floor. */
    				Attribute a = ((Floor)location).pickUpAttribute();

    				/* Check if we actually picked up something. */
    				if (a != null) {
    					attribute[index] = a;
    					return index;
    				}
    			}
    		}
    	}

    	return -1;
    }

    /**
     * Drop an attribute. Returns the attribute that was dropped
     * or null if the spot is not a floor spot or if there's already
     * an attribute on the floor.
     * 
     * @param attributeNr
     * @return boolean
     */
    public Attribute dropDown(int attributeNr) 
    {
    	if (!this.isDead()) {
    		/* Only drop on Floor spots. */
			if (location instanceof Floor) {
				Floor floor = (Floor)location;

				/* Check if there's already an attribute on the floor. */
				if (floor.getAttribute() == null) {
		        	Attribute a = attribute[attributeNr];

		        	/* Don't drop active attributes. */
		        	if (a != null && a.isActive())
		        		return null;

		        	/* Drop down the attribute. */
		        	floor.putAttribute(attribute[attributeNr]);
		        	attribute[attributeNr] = null;

		        	/* Return the attribute that was dropped. */
		        	return a;
		    	}
			}
    	}

		return null;
    }

    /**
     * Use an attribute. Returns the used attribute.
     * 
     * @param attributeNr
     * @return Attribute
     */
    public Attribute useAttribute(int attributeNr) 
    {
    	Attribute a = attribute[attributeNr];

    	if (!this.isDead() && a != null) {
    		/* Don't (re-)use active items. */
    		if (a.isActive())
    			return null;

    		/* Use the attribute. */
    		a.use(this);

    		/* Set it to null if the attribute is not active. */
			if (!a.isActive())
				setAttributeUsed(attributeNr);

    		return a;
    	}
    	return null;
    }

	/**
	 * Sets the attribute to used (null).
	 * 
	 * @param attributeNr
	 */
	public void setAttributeUsed(int attributeNr)
	{
		attribute[attributeNr] = null;
	}

    /**
     * Get attribute by index.
     * 
     * @param index
     * @return Attribute
     */
    public Attribute getAttributeByID(int index)
    {
    	return this.attribute[index];
    }

    /**
     * Get the index of an attribute.
     * 
     * @param a
     * @return int
     */
	public int getIDbyAttribute(Attribute a)
	{
		for (int i = 0; i < Constants.MAX_ATTRIBUTES_CARRIED; i++)
			if (this.attribute[i] == a)
				return i;
		return -1;
	}

    /**
     * Search for an active attribute a.
     * 
     * @param a
     * @return Attribute
     */
    public Attribute findActiveAttribute(char a)
    {
    	for (int i = 0; i < Constants.MAX_ATTRIBUTES_CARRIED; i++)
    		if (attribute[i] != null && attribute[i].getAttributeChar() == a && attribute[i].isActive())
    			return attribute[i];
    	return null;
    }

    /**
	 * Moves the character the CharacterSpot cs.
	 * Returns true on a succesful move.
	 * 
	 * @param cs
	 * @param changeAttributes
     * @param moves
	 * @return boolean
	 */
	public boolean moveTo(CharacterSpot cs, boolean changeAttributes, int moves)
	{
		 /*
		  * Character can move, is not kissing, is not dead,
		  * has energy and characterspot exists.
		  */
		if (canMove && !kissing && !isDead() && energy > 0 && cs != null) {
			CharacterSpot origLocation = cs, newLocation = cs;

			/* Check if we have enough energy to do the move. */
			if (!canMoveTo(newLocation))
			    return false;

			/*
			 * If we're moving onto a door, change the new location to
			 * a CharacterSpot near the other end of the teleporter.
			 */
			if (newLocation != null && newLocation instanceof Door) {
				Door d = ((Door)cs).getDoor();
				Room r = d.getRoom();

				newLocation = d;

				for (int i = Constants.UP_LEFT; i <= Constants.LEFT; i++) {
					CharacterSpot tmpCS = r.getSurroundingCharacterSpot(d, i);
					if (tmpCS != null &&
						(tmpCS.getCharacter() == null ||
						(tmpCS.getCharacter() != null && tmpCS.getCharacter().isDead())))
							if (newLocation == d || (newLocation != d &&
							    (i == Constants.UP || i == Constants.RIGHT ||
								 i == Constants.DOWN || i == Constants.LEFT))) 
									newLocation = tmpCS;
				}

				if (newLocation == d)
					return false;
			}

			if ((newLocation.getCharacter() == null ||
				(newLocation.getCharacter() != null && newLocation.getCharacter().isDead())))
			{
				if (location != null)
					location.setCharacter(null);
	
				newLocation.setCharacter(this);
				location = newLocation;

				if (changeAttributes && origLocation != null)
					moved(origLocation, moves);

				return true;
			}
		}

		return false;
	}

	/**
	 * Check if we have enough energy to do the move.
	 * 
	 * @param location
	 * @return boolean
	 */
	public boolean canMoveTo(CharacterSpot location)
	{
	    if (location instanceof Hole)
	        if ((this.getEnergy() - Constants.ENERGY_MOVEMENT_HOLE_DECR) <= 0)
	        	return false;
	    else if (location instanceof Door)
	        if ((this.getEnergy() - Constants.ENERGY_MOVEMENT_DOOR_DECR) <= 0)
	            return false;
	    else
	        if ((this.getEnergy() - Constants.ENERGY_MOVEMENT_FLOOR_DECR) <= 0)
	            return false;

	    return true;
	}

	/**
	 * Action after character has moved to another spot.
	 * - Lower the energy.
	 * 
	 * @param location
	 * @param moves
	 */
    public void moved(CharacterSpot location, int moves)
	{
        while (moves-- > 0) {
            if (location instanceof Hole)
                this.setEnergy(this.getEnergy() - Constants.ENERGY_MOVEMENT_HOLE_DECR);
            else if (location instanceof Door)
                this.setEnergy(this.getEnergy() - Constants.ENERGY_MOVEMENT_DOOR_DECR);
            else
                this.setEnergy(this.getEnergy() - Constants.ENERGY_MOVEMENT_FLOOR_DECR);
        }
	}

    /**
     * Action after character cloned.
     * - Halve the attribute values of both characters.
     * 
     * @param c
     */
    public void cloned(Character c)
    {
        /* Halve attributes of the current character. */
        health = (int)Math.ceil(health / 2);
    	energy = (int)Math.ceil(energy / 2);
    	strength = (int)Math.ceil(strength / 2);
    	speed = (int)Math.ceil(speed / 2);
    	intelligence = (int)Math.ceil(intelligence / 2);
    	armor = (int)Math.ceil(armor / 2);

    	/* Halve attributes of the clone. */
    	c.setHealth(health);
    	c.setEnergy(energy);
    	c.setStrength(strength);
    	c.setSpeed(speed);
    	c.setIntelligence(intelligence);
    	c.setArmor(armor);
    }

    /**
     * Starts the move timer.
     */
    public void startMoveTimer()
    {
    	int duration;
    	canMove = false;

		if (findActiveAttribute(Constants.ATTR_BROOMSTICK) != null)
			duration = 0;
		else
			duration = Dice.doThrowNumber(200 + 200 - speed * 2);

		moveTimer = new Timer(duration, 1, new MoveDelay(this));
	    moveTimer.start();
    }

    /*
     * To be implemented by subclasses.
     */
    public abstract Character getPlayerType();
    public abstract char getPlayerTypeChar();
    public abstract String getPlayerTypeName();
}
