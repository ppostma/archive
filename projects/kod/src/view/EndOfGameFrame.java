/* $Id: EndOfGameFrame.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * EndOfGameFrame.java
 * 
 * This frame is displayed after the game.
 */

package src.view;

import java.awt.Graphics;
import java.awt.Color;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class EndOfGameFrame extends JFrame implements ActionListener
{
	public static final int END = 0;
	public static final int WON = 1;
	public static final int LOST = 2;

	private int status;

	private JPanel contentPane;
	private JButton btnRestart = new JButton();
	private ImageIcon gameWon = new ImageIcon(ClassLoader.getSystemResource("images/won_game.jpg"));
	private ImageIcon gameLost = new ImageIcon(ClassLoader.getSystemResource("images/lost_game.jpg"));
	private ImageIcon gameEnd = new ImageIcon(ClassLoader.getSystemResource("images/end_game.jpg"));

	/**
	 * Constructor.
	 * 
	 * @param status
	 */
	public EndOfGameFrame(int status)
	{
		this.status = status;

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    setTitle("Kiss of Death :: End of Game");

	    contentPane = (JPanel)getContentPane();    
	    contentPane.setLayout(null);
	    contentPane.setBackground(Color.BLACK);

	    btnRestart.setBounds(new Rectangle(110, 290, 100, 20));
	    btnRestart.setText("Restart");
	    btnRestart.setVisible(true);
	    btnRestart.addActionListener(this);

	    contentPane.add(btnRestart);
	    contentPane.setVisible(true);

	    setSize(325, 355);
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

		if (this.status == WON)
			g.drawImage(gameWon.getImage(), 2, 18, null, null);
		else if (this.status == LOST)
			g.drawImage(gameLost.getImage(), 2, 18, null, null);
		else
			g.drawImage(gameEnd.getImage(), 2, 18, null, null);
	}

	/**
	 * Function that will be executed on an action (e.g. button press).
	 * 
	 * @param e
	 */
	public void actionPerformed(ActionEvent e)
	{
		if (e.getSource() == btnRestart) {
			this.setVisible(false);
			this.dispose();

			new SignInFrame();
		}	
	}
}

