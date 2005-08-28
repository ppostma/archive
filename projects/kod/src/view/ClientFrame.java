/* $Id: ClientFrame.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ClientFrame.java
 * 
 * The client frame.
 * Instances a PlayPanel, which draws the playfield.
 */

package src.view;

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.rmi.RemoteException;

import javax.swing.JFrame;
import javax.swing.JPanel;

import src.utils.Constants;
import src.watcher.Watchable;

public class ClientFrame extends JFrame implements KeyListener
{
	private PlayPanel pp;
	private Watchable g;
	private GameView gv;

	private boolean attacking;
	private boolean defending;
	private boolean dropping;
	private boolean kissing;
	private boolean running;
	private boolean cloning;

	private JPanel contentPane;

	/**
	 * Constructor.
	 * 
	 * @param game
	 * @param gameview
	 */
	public ClientFrame(Watchable game, GameView gameview)
	{
		super();
		attacking = false;
		defending = false;
		dropping = false;
		kissing = false;
		running = false;
		cloning = false;

		g = game;
		gv = gameview;

		pp = new PlayPanel(g, gv);
		pp.setLocation(0, 0);
		pp.setSize(838, 664);

		contentPane = (JPanel)getContentPane();
		contentPane.setLayout(null);
		contentPane.setVisible(true);
		contentPane.add(pp, null);

		addKeyListener(this);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setFocusable(true);
		setTitle("Kiss of Death");
		setResizable(false);
		setSize(838, 664);
		setVisible(true);
	}

	/**
	 * Will be called when a key is typed.
	 * 
	 * @param event
	 */
	public void keyTyped(KeyEvent event)
	{
	}

	/**
	 * Will be called when a key is pressed.
	 * 
	 * @param event
	 */
	public void keyPressed(KeyEvent event)
	{
		int direction = -1, attribute = -1;
		int keyCode = event.getKeyCode();

		switch (keyCode) {
		case KeyEvent.VK_NUMPAD8:
			direction = Constants.UP;
			break;
		case KeyEvent.VK_NUMPAD9:
			direction = Constants.UP_RIGHT;
			break;
		case KeyEvent.VK_NUMPAD7:
			direction = Constants.UP_LEFT;
			break;
		case KeyEvent.VK_NUMPAD2:
			direction = Constants.DOWN;
			break;
		case KeyEvent.VK_NUMPAD3:
			direction = Constants.DOWN_RIGHT;
			break;
		case KeyEvent.VK_NUMPAD1:
			direction = Constants.DOWN_LEFT;
			break;
		case KeyEvent.VK_NUMPAD4:
			direction = Constants.LEFT;
			break;
		case KeyEvent.VK_NUMPAD6:
			direction = Constants.RIGHT;
			break;
		case KeyEvent.VK_P:
			try {
                g.action_PickUpAttribute(gv.playerID, gv.activeCharacter);
            } catch (RemoteException e) {
            	System.err.println("RemoteException occured: " + e.getMessage());
            }
			break;
		case KeyEvent.VK_D:
		    dropping = true;
			break;
		case KeyEvent.VK_CONTROL:
			attacking = true;
			break;
		case KeyEvent.VK_K:
			kissing = true;
			break;
		case KeyEvent.VK_SPACE:
			running = true;
			break;
		case KeyEvent.VK_S:
			gv.switchCharacter();
			pp.updateViewAttributes();
			repaint();
			break;
		case KeyEvent.VK_C:
			cloning = true;
			break;
		case KeyEvent.VK_1:
		    attribute = 0;
			break;
		case KeyEvent.VK_2:
		    attribute = 1;
			break;
		case KeyEvent.VK_3:
		    attribute = 2;
			break;
		case KeyEvent.VK_4:
		    attribute = 3;
			break;
		case KeyEvent.VK_5:
		    attribute = 4;
			break;
		case KeyEvent.VK_6:
		    attribute = 5;
			break;
		case KeyEvent.VK_V:
			defending = true;			
			break;
		}

		if (direction != -1) {
			if (attacking)
                try {
                    g.action_Attack(gv.playerID, gv.activeCharacter, direction);
                } catch (RemoteException e1) {
                	System.err.println("RemoteException occured: " + e1.getMessage());
                }
            else if (running)
                try {
                    g.action_RunCharacter(gv.playerID, gv.activeCharacter, direction);
                } catch (RemoteException e2) {
                	System.err.println("RemoteException occured: " + e2.getMessage());
                }
            else if (kissing)
                try {
                    g.action_KissCharacter(gv.playerID, gv.activeCharacter, direction);
                } catch (RemoteException e3) {
                	System.err.println("RemoteException occured: " + e3.getMessage());
                }
            else if (cloning)
                try {
                    g.action_CloneCharacter(gv.playerID, direction);
                } catch (RemoteException e4) {
                	System.err.println("RemoteException occured: " + e4.getMessage());
                }
            else if (defending)
                try {
                    g.action_Defend(gv.playerID, gv.activeCharacter, direction);
                } catch (RemoteException e5) {
                	System.err.println("RemoteException occured: " + e5.getMessage());
                }
            else
                try {
                    g.action_MoveCharacter(gv.playerID, gv.activeCharacter, direction);
                } catch (RemoteException e6) {
                	System.err.println("RemoteException occured: " + e6.getMessage());
                }
		} else if (attribute != -1) {
		    if (dropping)
                try {
                    g.action_DropAttribute(gv.playerID, gv.activeCharacter, attribute);
                } catch (RemoteException e1) {
                	System.err.println("RemoteException occured: " + e1.getMessage());
                }
            else
                try {
                    g.action_UseAttribute(gv.playerID, gv.activeCharacter, attribute);
                } catch (RemoteException e2) {
                	System.err.println("RemoteException occured: " + e2.getMessage());
                }
		}
	}

	/**
	 * Will be called when a key is released.
	 * 
	 * @param event
	 */
	public void keyReleased(KeyEvent event)
	{
		int key = event.getKeyCode();

		switch (key) {
		case KeyEvent.VK_CONTROL:
			attacking = false;
			break;
		case KeyEvent.VK_K:
			kissing = false;
			break;
		case KeyEvent.VK_C:
			cloning = false;
			break;
		case KeyEvent.VK_SPACE:
			running = false;
			break;
		case KeyEvent.VK_D:
		    dropping = false;
			break;
		case KeyEvent.VK_V:
		    defending = false;
			break;
		}
	}

	/**
	 * Main, start of the program.
	 * 
	 * @param args
	 */
	public static void main(String[] args)
    {
        new SignInFrame();
    }
}
