/* $Id: Watcher.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * Watcher.java
 * 
 * Replacement for Observer.
 * This has to be implemented by the GameView class.
 * These events will be executed by the controller.
 */

package src.watcher;

import java.rmi.*;

public interface Watcher extends Remote
{
    void event_SignIn(int playerID, char[][][] spots) throws RemoteException;
	void event_NewPlayer(int playerID, String playerName, char playerType, char gender) throws RemoteException;
	void event_NewCharacter(int playerID, int characterNumber) throws RemoteException;
	void event_NewClone(int playerID) throws RemoteException;
	void event_NewMonster(int playerID) throws RemoteException;
	void event_RemovePlayer(int playerID) throws RemoteException;
	void event_CharacterMoved(int playerID, int characterNumber, int roomID, int x, int y) throws RemoteException;
	void event_CharacterDied(int deadPlayerID, int deadCharacterNumber) throws RemoteException;
	void event_CharacterGoingToAttack(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterAttack(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterAttacksNothing(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterGoingToDefend(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterDefend(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterGoingToKiss(int playerID, int characterNumber) throws RemoteException;
	void event_CharacterKissed(int playerID, int characterNumber, boolean frog) throws RemoteException;
	void event_CharacterHasBeenKissed(int playerID, int characterNumber, boolean frog) throws RemoteException;
	void event_CharacterAttributesUpdated(int playerID, int characterNumber, int health, int energy, int strength, int speed, int intelligence, int armor, boolean invisible, boolean nightvision) throws RemoteException;
	void event_CharacterPickedUp(int playerID, int characterNumber, int attributeID, char type) throws RemoteException;
	void event_CharacterDropped(int playerID, int characterNr, int attributeID) throws RemoteException;
	void event_CharacterUsed(int playerID, int characterNr, int attributeID) throws RemoteException;
	void event_CharacterUsedFinished(int playerID, int characterNr, int attributeID) throws RemoteException;
	void event_AttributePlaced(int roomID, char type, int xPos, int yPos) throws RemoteException;
	void event_AttributeRemoved(int roomID, int xPos, int yPos) throws RemoteException;
	void event_Msg(String message, String playername) throws RemoteException;
	void event_SendMessage(int type, String message, String playerName) throws RemoteException;
	void event_GameEvent(String message) throws RemoteException;
	void event_CountDown(int seconds) throws RemoteException;
	void event_StartGame() throws RemoteException;
	void event_EndGame() throws RemoteException;
	void event_WinGame() throws RemoteException;
	void event_LostGame() throws RemoteException;
}
