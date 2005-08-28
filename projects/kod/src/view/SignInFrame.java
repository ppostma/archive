/* $Id: SignInFrame.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * SignInFrame.java
 *
 * The sign in frame. 
 */

package src.view;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.List;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.rmi.Naming;
import java.rmi.RemoteException;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

import src.utils.Constants;
import src.watcher.Watchable;

public class SignInFrame extends JFrame implements ActionListener
{
	private JPanel contentPane;
	private JLabel lblName = new JLabel();
	private JLabel lblPlayerType = new JLabel();
	private JLabel lblGender = new JLabel();
	private JLabel lblMessage = new JLabel();
	private JTextField txtName = new JTextField();
	private JComboBox comboPlayerType = new JComboBox(new String[] {"Adventurer", "Dwarf", "Elf", "Warrior"});
	private JComboBox comboGender = new JComboBox(new String[] {"Male", "Female"});
	private JButton btnSignIn = new JButton();
	private List lstPlayers = new List();

	private JLabel labelHost = new JLabel();
	private JTextField textHost = new JTextField();
	private JButton btnConnect = new JButton();

	private ImageIcon background = new ImageIcon(ClassLoader.getSystemResource("images/loginnew.jpg"));

	private Watchable g;
	private GameView gv;

	private int playerID = -1;

	/**
	 * Constructor. 
	 * 
	 */
	public SignInFrame()
	{
		int x = 0, y = 180;

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    setTitle("Kiss of Death :: Connect");

	    contentPane = (JPanel)getContentPane();    
	    contentPane.setLayout(null);
	    contentPane.setBackground(Color.BLACK);

	    /* Connect stuff. */
	    labelHost.setBounds(new Rectangle(x+182, y+225, 170, 20));
	    labelHost.setText("Server address:");
	    labelHost.setForeground(Color.WHITE);
	    labelHost.setVisible(true);
	    
	    textHost.setBounds(new Rectangle(x+178, y+250, 100, 20));
	    textHost.setText("localhost");
	    textHost.setVisible(true);

	    btnConnect.setBounds(new Rectangle(x+178, y+277, 100, 20));
	    btnConnect.setText("Connect");
	    btnConnect.setVisible(true);
	    btnConnect.addActionListener(this);

	    /* Sign in stuff. */
	    lblName.setBounds(new Rectangle(x+95, y+223, 72, 18));
	    lblName.setText("Name:");
	    lblName.setForeground(Color.WHITE);
	    lblName.setVisible(false);

	    lblPlayerType.setBounds(new Rectangle(x+95, y+251, 73, 18));
	    lblPlayerType.setText("Player type:");
	    lblPlayerType.setForeground(Color.WHITE);
	    lblPlayerType.setVisible(false);

	    lblGender.setBounds(new Rectangle(x+95, y+281, 73, 18));
	    lblGender.setText("Gender:");
	    lblGender.setForeground(Color.WHITE);
	    lblGender.setVisible(false);

	    txtName.setBounds(new Rectangle(x+165, y+221, 187, 24));
	    txtName.setVisible(false);

	    comboPlayerType.setBounds(new Rectangle(x+165, y+251, 186, 24));
	    comboPlayerType.addActionListener(this);
	    comboPlayerType.setVisible(false);

	    comboGender.setBounds(new Rectangle(x+165, y+281, 80, 24));
	    comboGender.addActionListener(this);
	    comboGender.setVisible(false);

	    btnSignIn.setBounds(new Rectangle(x+270, y+281, 85, 24));
	    btnSignIn.setText("Sign in");
	    btnSignIn.addActionListener(this);
	    btnSignIn.setVisible(false);
	    
	    lstPlayers.setSize(150,133);
	    lstPlayers.setLocation(153, 367);
	    lstPlayers.setVisible(false);
	    
	    lblMessage.setSize(400, 20);
	    lblMessage.setLocation(25, lstPlayers.getY() + lstPlayers.getHeight());
	    lblMessage.setForeground(Color.WHITE);
		lblMessage.setHorizontalAlignment(SwingConstants.CENTER);
	    lblMessage.setVisible(false);

	    contentPane.add(lstPlayers);
	    contentPane.add(lblMessage);
	    contentPane.add(lblName);
	    contentPane.add(lblPlayerType);
	    contentPane.add(lblGender);
	    contentPane.add(txtName);
	    contentPane.add(labelHost);
	    contentPane.add(textHost);
	    contentPane.add(comboPlayerType);
	    contentPane.add(comboGender);
	    contentPane.add(btnSignIn);
	    contentPane.add(btnConnect);
	    contentPane.add(lstPlayers);
	    contentPane.add(lblMessage);
	    contentPane.setVisible(true);

	    setSize(456, 550);
	    setResizable(false);
		setVisible(true);

		repaint();
	}

	/**
	 * Paint function.
	 * 
	 * @param g
	 */
	public void paint(Graphics g)
	{  	
		super.paint(g);
		g.drawImage(background.getImage(), 3, 18, null, null);
	}

	/**
	 * Function that will be executed on an action (e.g. button press).
	 * 
	 * @param e
	 */
	public void actionPerformed(ActionEvent e)
	{
		if (e.getSource() == comboGender || e.getSource() == comboPlayerType) {
			repaint();
		}

		if (e.getSource() == btnConnect) {
		    /* Server text should not be empty. */
		    if (textHost.getText().length() < 1) {
	        	lblMessage.setText("Please enter a hostname or IP address.");
	        	lblMessage.setVisible(true);
	        	return;
		    }

		    btnConnect.setText("Please wait...");
		    btnConnect.setEnabled(false);
		    btnConnect.repaint();

		    try {
                /* Get the game via RMI lookup. */
                this.g = (Watchable)Naming.lookup("//" + textHost.getText() + "/" + Constants.GAME_NAME);
	        } catch (Exception ex) {
	        	lblMessage.setText("Unable to lookup: " + ex.getMessage());
	        	lblMessage.setVisible(true);

	        	btnConnect.setText("Connect");
			    btnConnect.setEnabled(true);
	        	return;
	        }
	        lblMessage.setText("");

	        /* Create an instance of the GameView. */
	        try {
	            this.gv = new GameView(this.g);
	        } catch (RemoteException ex) {
	            System.err.println("Unable to instance GameView: " + ex.getMessage());
	            System.exit(1);
	        }

	        gv.setSignInFrame(this);

	        /* Set Connect stuff to invisible. */
		    labelHost.setVisible(false);
		    textHost.setVisible(false);
		    btnConnect.setVisible(false);

		    /* Set sign in stuff to visible. */
		    lblName.setVisible(true);
		    lblPlayerType.setVisible(true);
		    lblGender.setVisible(true);
		    txtName.setVisible(true);
		    comboPlayerType.setVisible(true);
		    comboGender.setVisible(true);
		    btnSignIn.setVisible(true);

		    setTitle("Kiss of Death :: Sign in");

		} else if (e.getSource() == btnSignIn) {
	        /* Playername should not be empty. */
	        if (txtName.getText().length() < 1) {
	        	lblMessage.setText("Please enter a name.");
	        	lblMessage.setVisible(true);
	        	return;
	        }

	        btnSignIn.setEnabled(false);

	        char playerType = 0;
		    char gender = 0;
		    int index = comboPlayerType.getSelectedIndex();

		    /* Select the PlayerType. */
		    if (comboPlayerType.getItemAt(index) == Constants.ADVENTURER_NAME)
		        playerType = Constants.ADVENTURER;
		    else if (comboPlayerType.getItemAt(index) == Constants.DWARF_NAME)
		        playerType = Constants.DWARF;
		    else if (comboPlayerType.getItemAt(index) == Constants.ELF_NAME)    
		        playerType = Constants.ELF;
	        else if (comboPlayerType.getItemAt(index) == Constants.WARRIOR_NAME)
	        	playerType = Constants.WARRIOR;

		    index = comboGender.getSelectedIndex();

		    /* Select the Gender. */
	        if (comboGender.getItemAt(index) == Constants.MALE_NAME)
	            gender = Constants.MALE;
	        else if (comboGender.getItemAt(index) == Constants.FEMALE_NAME)
	            gender = Constants.FEMALE;

	        /* Send the controller a "sign in" message. */ 
			try {
				playerID = g.action_SignIn(gv, txtName.getText(), playerType, gender);
			} catch (RemoteException re) {
			    System.err.println("RemoteException in action_SignIn: " + re.getMessage());
			    System.exit(1);
			}

			/* SignIn can return -1 if the game is full. */
	        if (playerID != -1) {
	            /* Set sign in stuff to invisible. */
			    lblName.setVisible(false);
			    lblPlayerType.setVisible(false);
			    lblGender.setVisible(false);
			    txtName.setVisible(false);
			    comboPlayerType.setVisible(false);
			    comboGender.setVisible(false);
			    btnSignIn.setVisible(false);

			    lstPlayers.setVisible(true);
			    lblMessage.setText("Waiting for other players...");
			    lblMessage.setVisible(true);

			    setTitle("Kiss of Death :: Waiting");

				/* Add a shutdown hook to sign off when closing the window. */
		        class shutdownHook extends Thread
				{
				    private Watchable game;
				    private GameView view;

				    public shutdownHook(Watchable g, GameView gv)
				    {
				        this.game = g;
				        this.view = gv;
				    }

				    public void run()
				    {
				        if (view.playerID != -1) {
					        try {
			                    game.action_SignOff(view.playerID);
			                } catch (RemoteException e) {
			                    System.err.println("Failed to sign off: " + e.getMessage());
			                }
				        }
				    }
				}

				Runtime.getRuntime().addShutdownHook(new shutdownHook(g, gv));
	        } else {
	        	lblMessage.setText("Cannot sign in: the game is already active.");
	        	lblMessage.setVisible(true);
	        }
        }
	}

	/**
	 * Updates the count down.
	 * 
	 * @param seconds
	 */
	public void updateCountDown(int seconds)
	{
		if (seconds == -1)
			lblMessage.setText("Waiting for other players...");
		else
			lblMessage.setText(seconds + " seconds remaining...");
	}

	/**
	 * Refresh the list of signed in players.
	 */
	public void refreshPlayers()
	{
        lstPlayers.removeAll();

	    /* Show all players in the list. */
	    for (int i = 0; i < Constants.MAX_PLAYERS; i++)
			if (gv.players[i] != null)
			    addPlayer(gv.players[i].name, gv.players[i].playerType);
	}

	/**
     * Adds a player to the list.
     * 
     * @param name
     * @param type
     */
    private void addPlayer(String name, char type)
    {
        switch (type){
	    case Constants.ADVENTURER:
	    	lstPlayers.add("[" + Constants.ADVENTURER_NAME + "] " + name);
	    	break;
		case Constants.DWARF:
			lstPlayers.add("[" + Constants.DWARF_NAME + "] " + name);
			break;
		case Constants.WARRIOR:
			lstPlayers.add("[" + Constants.WARRIOR_NAME + "] " + name);
			break;
		case Constants.ELF:
			lstPlayers.add("[" + Constants.ELF_NAME + "] " + name);
			break;
		}
    }
}
