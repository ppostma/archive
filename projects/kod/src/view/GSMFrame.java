/* $Id: GSMFrame.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * GSMFrame.java
 *
 * The frame for direct chatting with other players 
 */

package src.view;

import java.awt.Graphics;
import java.awt.List;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.rmi.RemoteException;
import java.sql.Time;
import java.util.ArrayList;
import java.util.Date;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;
import javax.swing.WindowConstants;

import src.utils.Constants;
import src.watcher.Watchable;

public class GSMFrame extends JFrame implements ActionListener, KeyListener
{
	private JPanel contentPane;
	private JButton btnSend = new JButton();
	private List lstPlayers = new List();	
	private ArrayList lstPlayerIDs = new ArrayList();
	private JTextArea chatOutput;
	private JTextField chatInput;
	private Watchable g;
	private GameView gv;

	/**
	 * Constructor. 
	 * 
	 * @param game
	 * @param gameview
	 */
	public GSMFrame(Watchable game, GameView gameview)
	{
		this.g = game;
		this.gv = gameview;
		setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
	    setTitle("Kiss of Death :: GSM Frame");	   

	    contentPane = (JPanel)getContentPane();
	    contentPane.setVisible(true);
	    contentPane.setLayout(null);

	    chatOutput = new JTextArea();
		chatOutput.setEditable(false);
		chatOutput.setLineWrap(true);
		chatOutput.setWrapStyleWord(true);
		chatOutput.setVisible(true);		
		JScrollPane spChatOutput = 
			new JScrollPane(chatOutput,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		spChatOutput.setBounds(new Rectangle(90, 10, 500, 150));
		spChatOutput.setSize(400, 180);		
		spChatOutput.setAutoscrolls(true);

		lstPlayers.setBounds(new Rectangle(500, 10, 120, 210));	 
		for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
			if (gv.players[i] != null) {
				lstPlayers.add(gv.players[i].name);
				lstPlayerIDs.add(new Integer(i));
			}
		}

		chatInput = new JTextField();
		chatInput.setVisible(true);
		chatInput.setSize(325,20);
		chatInput.setLocation(90, 200);

		btnSend.setLocation(425, 200);
	    btnSend.setText("Send");
	    btnSend.setSize(65,20);
	    btnSend.addActionListener(this);

	    contentPane.add(btnSend, null);
	    contentPane.add(lstPlayers, null);
	    contentPane.add(spChatOutput, null);
	    contentPane.add(chatInput, null);
		chatInput.addKeyListener(this);

	    setResizable(false);
	    setSize(640, 261);	   
	    setVisible(true);
	    repaint();

	    chatInput.requestFocusInWindow();
	}

	public void paint(Graphics g)
	{
    	super.paint(g);
	}

	public void msg(String sMessage, String sPlayerName)
	{
		Time t = new Time(new Date().getTime());
		chatOutput.append("[" + t.toString() + "] " + sPlayerName + ": " + sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}

	/**
	 * Function that will be executed on an action (e.g. button press).
	 * 
	 * @param event
	 */
	public void actionPerformed(ActionEvent event)
	{
	    /* Check for button press on "Send". */
        if (event.getSource() == btnSend && !chatInput.getText().equals("") && lstPlayers.getSelectedIndex() != -1) {
    		int playerID = ((Integer)lstPlayerIDs.get(lstPlayers.getSelectedIndex())).intValue();

        	if (gv.playerID == playerID) {
    			chatOutput.append("You cannot send a message to yourself.\n");
    		} else {
    			try {
					g.action_Msg(gv.playerID, playerID, chatInput.getText());
				} catch (RemoteException e) {
					System.err.println("RemoteException in action_Msg: " + e.getMessage());
				}
    			Time t = new Time(new Date().getTime());
    			chatOutput.append("[" + t.toString() + "] You to " + gv.players[playerID].name + ": "+ chatInput.getText() + "\n");
    		}
        	chatInput.setText("");
        	chatInput.requestFocusInWindow();
        }
	}

	public void keyTyped(KeyEvent event)
	{
	}

	public void keyPressed(KeyEvent event)
	{
		if (event.getKeyCode() == KeyEvent.VK_ENTER) {
	        if (!chatInput.getText().equals("") && lstPlayers.getSelectedIndex() != -1) {
	        	int playerID = ((Integer)lstPlayerIDs.get(lstPlayers.getSelectedIndex())).intValue();

        		if (gv.playerID == playerID) {
        			chatOutput.append("You cannot send a message to yourself.\n");
        		} else {
	                try {
						g.action_Msg(gv.playerID, playerID, chatInput.getText());
					} catch (RemoteException e) {
						System.err.println("RemoteException in action_Msg: " + e.getMessage());
					}
	                Time t = new Time(new Date().getTime());
	        		chatOutput.append("[" + t.toString() + "] You to " + gv.players[playerID].name + ": "+ chatInput.getText() + "\n");
        		}
	        	chatInput.setText("");
	        	chatInput.requestFocusInWindow();
	        }
		}
	}

	public void keyReleased(KeyEvent event)
	{
	}
}
