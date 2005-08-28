/* $Id: GameView.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * GameView.java
 * 
 * GameView is the class which "observes"/"watches" the controller.
 * - This currently refreshes the PlayPanel.
 */

package src.view;

import java.awt.Point;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

import src.utils.Constants;
import src.watcher.Watchable;
import src.watcher.Watcher;

public class GameView extends UnicastRemoteObject implements Watcher
{
	private Watchable g;
	private SignInFrame sf;
	private PlayPanel pp;
	private GSMFrame gsm;
	private ClientFrame cf;

	public int activeCharacter;
	public int playerID;
	public char[][][] spot; // [roomID][x][y]
	public ViewPlayer[] players = new ViewPlayer[Constants.MAX_PLAYERS];
	public ViewAttribute[] viewAttributes = new ViewAttribute[Constants.MAX_ATTRIBUTES * Constants.MAX_ATTRIBUTES];

	/**
	 * Constructor.
	 * 
	 * @param g
	 * @throws RemoteException
	 */
	public GameView(Watchable g) throws RemoteException
	{
		super();

		this.playerID = -1;
		this.g = g;
	}

	/**
	 * Sets the attribute pp of the type PlayPanel to the parameter.
	 * 
	 * @param pp
	 */
	public void setPlayPanel(PlayPanel pp)
	{
		this.pp = pp;
	}

	/**
	 * Sets the sign in frame.
	 *  
	 * @param sf
	 */
	public void setSignInFrame(SignInFrame sf)
	{
	    this.sf = sf;
	}

	/**
	 * Repaint the PlayPanel.
	 */
	private void repaintPlayPanel()
	{
		if (pp != null)
			pp.repaint();
	}

	/**
	 * Refresh the player list in the SignIn Frame.
	 */
	private void refreshSignInFrame()
	{
		if (sf != null)
			sf.refreshPlayers();
	}

	/**
	 * This method switches the active character.
	 */
	public void switchCharacter()
	{
		int character = 1 - activeCharacter;

		if (players[playerID].character[character] == null || players[playerID].character[character].dead)
			return;
		activeCharacter = character;

		repaintPlayPanel();
	}

	/*
	 * Implement the interface.
	 */
	public void event_SignIn(int playerID, char[][][] spots)
    {
        this.playerID = playerID;
        this.spot = spots;
    }

    public void event_NewPlayer(int playerID, String playerName, char playerType, char gender) throws RemoteException
	{
		players[playerID] = new ViewPlayer(playerName, playerType, gender);
		repaintPlayPanel();
		refreshSignInFrame();
	}

	public void event_NewCharacter(int playerID, int characterNr) throws RemoteException
	{
		players[playerID].character[characterNr] = new ViewCharacter(players[playerID].character[0].playerType, players[playerID].character[0].gender);
		repaintPlayPanel();
	}

	public void event_NewClone(int playerID) throws RemoteException
	{
		ViewPlayer p = players[playerID];
		p.character[1] = new ViewCharacter(p.character[0].playerType, p.character[0].gender);
		repaintPlayPanel();
	}

	public void event_NewMonster(int playerID) throws RemoteException
	{
		ViewPlayer p = players[playerID];
		p.monster = true;
	}

	public void event_RemovePlayer(int playerID) throws RemoteException
	{
		players[playerID] = null;
		repaintPlayPanel();
		refreshSignInFrame();
	}

	public void event_CharacterMoved(int playerID, int characterNr, int roomID, int x, int y) throws RemoteException
	{
		players[playerID].character[characterNr].roomID = roomID;
		players[playerID].character[characterNr].pos = new Point(x, y);
		repaintPlayPanel();
	}

	public void event_CharacterDied(int playerID, int characterNr) throws RemoteException
	{
		players[playerID].character[characterNr].dead = true;
		repaintPlayPanel();
	}

    public void event_CharacterGoingToAttack(int playerID, int characterNr) throws RemoteException
    {
        players[playerID].character[characterNr].attacking = true;
        repaintPlayPanel();
    }

    public void event_CharacterAttack(int playerID, int characterNr) throws RemoteException
	{
        players[playerID].character[characterNr].attacking = false;
		repaintPlayPanel();
	}

	public void event_CharacterAttacksNothing(int playerID, int characterNr) throws RemoteException
	{
		players[playerID].character[characterNr].attacking = false;
		repaintPlayPanel();
	}

    public void event_CharacterGoingToDefend(int playerID, int characterNr) throws RemoteException
    {
        players[playerID].character[characterNr].defending = true;
        repaintPlayPanel();
    }

    public void event_CharacterDefend(int playerID, int characterNr) throws RemoteException
	{
        players[playerID].character[characterNr].defending = false;
		repaintPlayPanel();
	}

	public void event_CharacterGoingToKiss(int playerID, int characterNr) throws RemoteException
	{
		players[playerID].character[characterNr].kissing = true;
		repaintPlayPanel();
	}

	public void event_CharacterKissed(int playerID, int characterNr, boolean frog) throws RemoteException
	{
		players[playerID].character[characterNr].kissing = false;
		players[playerID].character[characterNr].frog = frog;
		repaintPlayPanel();
	}

    public void event_CharacterHasBeenKissed(int playerID, int characterNr, boolean frog) throws RemoteException
    {
		players[playerID].character[characterNr].frog = frog;
		repaintPlayPanel();
    }

    public void event_CharacterAttributesUpdated(int playerID, int characterNr, int health, int energy, int strength, int speed, int intelligence, int armor, boolean invisible, boolean nightvision) throws RemoteException
	{
		ViewCharacter c = players[playerID].character[characterNr];

		c.health = health;
		c.energy = energy;
		c.strength = strength;
		c.intelligence = intelligence;
		c.speed = speed;
		c.armor = armor;
		c.nightvision = nightvision;
		c.invisible = invisible;

		if (pp != null)
			pp.updateViewAttributes();		
	}

	public void event_CharacterPickedUp(int playerID, int characterNr, int attributeID, char type) throws RemoteException
    {
        ViewCharacter ch = players[playerID].character[characterNr];
        ch.attribute[attributeID] = new ViewUseAttribute(type);
    }

    public void event_CharacterDropped(int playerID, int characterNr, int attributeID) throws RemoteException
    {
        ViewCharacter ch = players[playerID].character[characterNr];
        ch.attribute[attributeID] = null;
    }

	public void event_CharacterUsed(int playerID, int characterNr, int attributeID) throws RemoteException
	{
		ViewCharacter ch = players[playerID].character[characterNr];
		ch.attribute[attributeID].active = true;
	}
	
	public void event_CharacterUsedFinished(int playerID, int characterNr, int attributeID) throws RemoteException
	{
		ViewCharacter ch = players[playerID].character[characterNr];
		ch.attribute[attributeID] = null;
	}

	public void event_AttributePlaced(int roomID, char type, int xPos, int yPos) throws RemoteException
	{
	    int attributeID = -1;

	    for (int i = 0; i < Constants.MAX_ATTRIBUTES * Constants.MAX_ATTRIBUTES; i++) {
	    	ViewAttribute va = viewAttributes[i]; 

	    	/* Check if the attribute exists in the viewAttributes array. */
	    	if (va != null && va.type == type && va.pos.x == xPos && va.pos.y == yPos)
	    		break;
	    	/* Check if this element is free. */
	    	if (va == null) {
	    	    attributeID = i;
	            break;
	        }
	    }

	    if (attributeID != -1) {
	    	viewAttributes[attributeID] = new ViewAttribute(roomID, type, new Point(xPos, yPos));
	    	repaintPlayPanel();
	    }
    }

    public void event_AttributeRemoved(int roomID, int xPos, int yPos) throws RemoteException
    {
        int attributeID = -1;

        /* Check which attribute has been picked up. */
	    for (int i = 0; i < Constants.MAX_ATTRIBUTES * Constants.MAX_ATTRIBUTES; i++) {
	        ViewAttribute va = viewAttributes[i]; 
	        if (va != null && va.roomID == roomID && va.pos.x == xPos && va.pos.y == yPos) {
	    	   	attributeID = i;
	    	   	break;
	        }
	    }    

	    if (attributeID != -1) {
	    	viewAttributes[attributeID] = null;
	    	repaintPlayPanel();
	    }
	}

    public void event_Msg(String message, String playerName) throws RemoteException
	{
		if (pp != null) {
			gsm = pp.getGSM();
			if (gsm != null)
				gsm.msg(message, playerName);
			pp.gsm(message, playerName);
		}
	}

	public void event_SendMessage(int type, String message, String playerName)
	{
		if (pp != null) {
			if (type == Constants.ACTION_TEAMCHAT) {
				pp.teamChat(message, playerName);
			} else if (type == Constants.ACTION_YELL) {
				pp.yell(message, playerName);
			} else if (type == Constants.ACTION_CHAT) {
				pp.chat(message, playerName);
			}
			pp.transferFocusUpCycle();
		}
	}

	public void event_GameEvent(String message) throws RemoteException
	{
		if (pp != null)
			pp.gameEvent("---> " + message);
	}

	public void event_CountDown(int seconds) throws RemoteException
	{
		if (sf != null)
			sf.updateCountDown(seconds);
	}

	public void event_StartGame() throws RemoteException
	{
    	this.cf = new ClientFrame(this.g, this);

    	/* Get rid of the sign in frame. */
    	sf.setVisible(false);
    	sf.dispose();
    	sf = null;
	}

	public void event_EndGame() throws RemoteException
	{
	    cf.setVisible(false);
	    cf.dispose();

	    new EndOfGameFrame(EndOfGameFrame.END);
	}

	public void event_WinGame() throws RemoteException
	{
	    cf.setVisible(false);
	    cf.dispose();

	    new EndOfGameFrame(EndOfGameFrame.WON);
	}

	public void event_LostGame() throws RemoteException
	{
	    cf.setVisible(false);
	    cf.dispose();
	    
	    new EndOfGameFrame(EndOfGameFrame.LOST);
	}
}
