/* $Id: PlayPanel.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * PlayPanel.java
 * 
 * The PlayPanel class is the panel for the bottom panel which contains
 * the energy-, health-, speed- and intelligencebar 
 */

package src.view;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.rmi.RemoteException;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;

import src.utils.Constants;
import src.watcher.Watchable;

public class PlayPanel extends JPanel implements ActionListener, KeyListener
{
	private Watchable g;
	private GameView gv;

	// Chat.
	private JButton btnChatWhisper;
	private JTextArea chatOutput;
	private JTextField chatInput;
	private JButton btnChatYell;
	private JButton btnChatTeam;
	private JButton btnChatAll;

	private GSMFrame gsm;

	// Graphics playfield.
	private ImageIcon wall = new ImageIcon(ClassLoader.getSystemResource("images/wall01.gif"));
	private ImageIcon door = new ImageIcon(ClassLoader.getSystemResource("images/door01.gif"));
	private ImageIcon floor = new ImageIcon(ClassLoader.getSystemResource("images/floor.gif"));
	private ImageIcon hole = new ImageIcon(ClassLoader.getSystemResource("images/hole.gif"));

	private ImageIcon armor = new ImageIcon(ClassLoader.getSystemResource("images/armor.gif"));
	private ImageIcon armor_active = new ImageIcon(ClassLoader.getSystemResource("images/armor_active.gif"));
	private ImageIcon broomstick = new ImageIcon(ClassLoader.getSystemResource("images/broomstick.gif"));
	private ImageIcon broomstick_active = new ImageIcon(ClassLoader.getSystemResource("images/broomstick_active.gif"));
	private ImageIcon invisibility = new ImageIcon(ClassLoader.getSystemResource("images/invisibility.gif"));
	private ImageIcon invisibility_active = new ImageIcon(ClassLoader.getSystemResource("images/invisibility_active.gif"));
	private ImageIcon lipstick = new ImageIcon(ClassLoader.getSystemResource("images/lipstick.gif"));
	private ImageIcon lipstick_active = new ImageIcon(ClassLoader.getSystemResource("images/lipstick_active.gif"));
	private ImageIcon nightvision = new ImageIcon(ClassLoader.getSystemResource("images/nightvision.gif"));
	private ImageIcon nightvision_active = new ImageIcon(ClassLoader.getSystemResource("images/nightvision_active.gif"));
	private ImageIcon shield = new ImageIcon(ClassLoader.getSystemResource("images/shield.gif"));
	private ImageIcon shield_active = new ImageIcon(ClassLoader.getSystemResource("images/shield_active.gif"));
	private ImageIcon sword = new ImageIcon(ClassLoader.getSystemResource("images/sword.gif"));
	private ImageIcon sword_active = new ImageIcon(ClassLoader.getSystemResource("images/sword_active.gif"));
	private ImageIcon vitamines = new ImageIcon(ClassLoader.getSystemResource("images/vitamines.gif"));

	private ImageIcon adventurer = new ImageIcon(ClassLoader.getSystemResource("images/adventurer.gif"));
	private ImageIcon dwarf = new ImageIcon(ClassLoader.getSystemResource("images/dwarf.gif"));
	private ImageIcon elf = new ImageIcon(ClassLoader.getSystemResource("images/elf.gif"));
	private ImageIcon warrior = new ImageIcon(ClassLoader.getSystemResource("images/warrior.gif"));
	private ImageIcon monster = new ImageIcon(ClassLoader.getSystemResource("images/monster.gif"));
	private ImageIcon dead = new ImageIcon(ClassLoader.getSystemResource("images/dead.gif"));
	private ImageIcon frog = new ImageIcon(ClassLoader.getSystemResource("images/frog.gif"));

	private ImageIcon adventurer_kiss = new ImageIcon(ClassLoader.getSystemResource("images/adventurer_kiss.gif"));
	private ImageIcon dwarf_kiss = new ImageIcon(ClassLoader.getSystemResource("images/dwarf_kiss.gif"));
	private ImageIcon elf_kiss = new ImageIcon(ClassLoader.getSystemResource("images/elf_kiss.gif"));
	private ImageIcon warrior_kiss = new ImageIcon(ClassLoader.getSystemResource("images/warrior_kiss.gif"));

	private ImageIcon adventurer_def = new ImageIcon(ClassLoader.getSystemResource("images/adventurer_def.gif"));
	private ImageIcon dwarf_def = new ImageIcon(ClassLoader.getSystemResource("images/dwarf_def.gif"));
	private ImageIcon elf_def = new ImageIcon(ClassLoader.getSystemResource("images/elf_def.gif"));
	private ImageIcon warrior_def = new ImageIcon(ClassLoader.getSystemResource("images/warrior_def.gif"));
	private ImageIcon monster_def = new ImageIcon(ClassLoader.getSystemResource("images/monster_def.gif"));

	private ImageIcon adventurer_att = new ImageIcon(ClassLoader.getSystemResource("images/adventurer_att.gif"));
	private ImageIcon dwarf_att = new ImageIcon(ClassLoader.getSystemResource("images/dwarf_att.gif"));
	private ImageIcon elf_att = new ImageIcon(ClassLoader.getSystemResource("images/elf_att.gif"));
	private ImageIcon warrior_att = new ImageIcon(ClassLoader.getSystemResource("images/warrior_att.gif"));
	private ImageIcon monster_att = new ImageIcon(ClassLoader.getSystemResource("images/monster_att.gif"));

	private ImageIcon intface = new ImageIcon(ClassLoader.getSystemResource("images/interface.gif"));
	private ImageIcon chatGsm = new ImageIcon(ClassLoader.getSystemResource("images/gsm.gif"));
	private ImageIcon chatAll = new ImageIcon(ClassLoader.getSystemResource("images/all.gif"));
	private ImageIcon chatYell = new ImageIcon(ClassLoader.getSystemResource("images/yell.gif"));
	private ImageIcon chatTeam = new ImageIcon(ClassLoader.getSystemResource("images/team.gif"));

	private ImageIcon st_adventurer = new ImageIcon(ClassLoader.getSystemResource("images/st_adventurer.gif"));
	private ImageIcon st_dwarf = new ImageIcon(ClassLoader.getSystemResource("images/st_dwarf.gif"));
	private ImageIcon st_elf = new ImageIcon(ClassLoader.getSystemResource("images/st_elf.gif"));
	private ImageIcon st_warrior = new ImageIcon(ClassLoader.getSystemResource("images/st_warrior.gif"));
	private ImageIcon st_monster = new ImageIcon(ClassLoader.getSystemResource("images/st_monster.gif"));

	/**
	 * Constructor.
	 * 
	 * @param game
	 * @param gameview
	 */
	public PlayPanel(Watchable game, GameView gameview)
	{
		setLayout(null);

		g = game;
		gv = gameview;
		gv.setPlayPanel(this); 

		Point pntChat = new Point(200, 516);

		chatOutput = new JTextArea();
		chatOutput.setEditable(false);
		chatOutput.setLineWrap(true);
		chatOutput.setWrapStyleWord(true);
		chatOutput.setBackground(Color.BLACK);
		chatOutput.setForeground(Color.WHITE);
		chatOutput.setVisible(true);

		JScrollPane spChatOutput = 
			new JScrollPane(chatOutput,
			    ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS,
			    ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

		spChatOutput.setLocation(pntChat.x+62,pntChat.y+31);
		spChatOutput.setSize(359, 50);
		spChatOutput.setBackground(Color.BLACK);
		chatInput = new JTextField();
		chatInput.setBackground(Color.BLACK);
		chatInput.setForeground(Color.WHITE);
		chatInput.setBorder(null);
		chatInput.setVisible(true);
		chatInput.setSize(360,22);
		chatInput.setLocation(pntChat.x+61,pntChat.y+90);
		chatInput.addKeyListener(this);
		
		btnChatWhisper = new JButton(chatGsm);
		btnChatWhisper.setVisible(true);
		btnChatWhisper.setSize(60,23);
		btnChatWhisper.setBorder(null);
		btnChatWhisper.setLocation(630,541);
		btnChatWhisper.addActionListener(this);
		
		btnChatTeam = new JButton(chatTeam);
		btnChatTeam.setVisible(true);
		btnChatTeam.setSize(60,23);
		btnChatTeam.setLocation(630,564);
		btnChatTeam.setBorder(null);
		btnChatTeam.addActionListener(this);
 
		btnChatYell = new JButton(chatYell);
		btnChatYell.setVisible(true);
		btnChatYell.setSize(60,23);
		btnChatYell.setLocation(630,587);
		btnChatYell.setBorder(null);
		btnChatYell.addActionListener(this);
		
		btnChatAll = new JButton(chatAll);
		btnChatAll.setVisible(true);
		btnChatAll.setSize(60,23);
		btnChatAll.setLocation(630,610);
		btnChatAll.setBorder(null);
		btnChatAll.addActionListener(this);

		add(spChatOutput);
		add(chatInput);
		add(btnChatWhisper);
		add(btnChatAll);
		add(btnChatYell);
		add(btnChatTeam);

		updateViewAttributes();
	}

	/**
	 * Paint function.
	 * 
	 * @param g
	 */
	public void paint(Graphics g)
	{
    	super.paint(g);

    	ViewCharacter cc = gv.players[gv.playerID].character[gv.activeCharacter];
    	int roomID = cc.roomID;
    	int ss = Constants.SPOT_SIZE;

    	g.drawImage(intface.getImage(),0,512,this);

    	// Iterate through all spots in the current room.
    	for (int y = 0; y < Constants.ROOM_HEIGHT; y++) {
			for (int x = 0; x < Constants.ROOM_WIDTH; x++) {
				char cSpot = gv.spot[roomID][x][y];
				switch (cSpot){
				case Constants.DOOR:
					g.drawImage(door.getImage(),x*ss,y*ss,this);
					break;
				case Constants.WALL:
					g.drawImage(wall.getImage(),x*ss,y*ss,this);
					break;
				case Constants.HOLE:
					g.drawImage(hole.getImage(),x*ss,y*ss,this);					
					break;
				case Constants.FLOOR:
					g.drawImage(floor.getImage(),x*ss,y*ss,this);
					break;
				default:
					g.setColor(Color.WHITE);
					g.fillRect(x*ss, y*ss, ss-1, ss-1);
					break;
				}

				// Draw the attributes in the playfield.
				for (int i = 0; i < Constants.MAX_ATTRIBUTES * Constants.MAX_ATTRIBUTES; i++) {
					if (gv.viewAttributes[i] != null) {
					    ViewAttribute va = gv.viewAttributes[i];
						if (va.roomID == roomID && va.pos.x == x && va.pos.y == y) {
							switch (va.type) {
							case Constants.ATTR_ARMOR:
								g.drawImage(armor.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_BROOMSTICK:
								g.drawImage(broomstick.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_INVISIBILITY:
								g.drawImage(invisibility.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_LIPSTICK:
								g.drawImage(lipstick.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_NIGHTVISION:
								g.drawImage(nightvision.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_SHIELD:
								g.drawImage(shield.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_SWORD:
								g.drawImage(sword.getImage(),x*ss,y*ss,this);
								break;
							case Constants.ATTR_VITAMINES:
								g.drawImage(vitamines.getImage(),x*ss,y*ss,this);
								break;
							}
						}
					}
				}

				// Draw dead bodys.
				for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
					if (gv.players[i] != null) {
						for (int j = 0; j < Constants.MAX_CHARACTERS; j++) {
							ViewCharacter vc = gv.players[i].character[j];
							if (vc != null && vc.dead && vc.pos.x == x && vc.pos.y == y && vc.roomID == roomID)
								g.drawImage(dead.getImage(),x*ss,y*ss,this);
						}
					}
				}

				// Draw living characters.
				for (int i = 0; i < Constants.MAX_PLAYERS; i++) {
					if (gv.players[i] != null) {
						for (int j = 0; j < Constants.MAX_CHARACTERS; j++) {
							ViewCharacter vc = gv.players[i].character[j];
							if (vc != null && vc.pos.x == x && vc.pos.y == y && vc.roomID == roomID) {
								boolean bInvisible = vc.invisible && !cc.nightvision && i != gv.playerID;

								if (vc.frog == true) {
									g.drawImage(frog.getImage(),x*ss,y*ss,this);
								} else if (vc.kissing == true) {
									if (vc.playerType == Constants.ADVENTURER)
										g.drawImage(adventurer_kiss.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.DWARF)
										g.drawImage(dwarf_kiss.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ELF)
										g.drawImage(elf_kiss.getImage(),x*ss,y*ss,this);
									else
										g.drawImage(warrior_kiss.getImage(),x*ss,y*ss,this);
								} else if (vc.attacking == true) { 
									if (gv.players[i].monster)
										g.drawImage(monster_att.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ADVENTURER)
										g.drawImage(adventurer_att.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.DWARF)
										g.drawImage(dwarf_att.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ELF)
										g.drawImage(elf_att.getImage(),x*ss,y*ss,this);
									else
										g.drawImage(warrior_att.getImage(),x*ss,y*ss,this);
								} else if (vc.defending == true) { 
									if (gv.players[i].monster)
										g.drawImage(monster_def.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ADVENTURER)
										g.drawImage(adventurer_def.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.DWARF)
										g.drawImage(dwarf_def.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ELF)
										g.drawImage(elf_def.getImage(),x*ss,y*ss,this);
									else
										g.drawImage(warrior_def.getImage(),x*ss,y*ss,this);
								} else if (!bInvisible && !vc.dead) {
									g.setColor(Color.RED);
									if (gv.players[i].monster)
										g.drawImage(monster.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.DWARF)
										g.drawImage(dwarf.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.WARRIOR)
										g.drawImage(warrior.getImage(),x*ss,y*ss,this);
									else if (vc.playerType == Constants.ADVENTURER)
										g.drawImage(adventurer.getImage(),x*ss,y*ss,this);
									else
										g.drawImage(elf.getImage(),x*ss,y*ss,this);
								}
							}
						}
					}
				}
			}
		}

    	// Draw the attribute bars.
    	int max = (gv.players[gv.playerID].monster ? 119 : 100);
    	g.setColor(Color.RED);
    	g.fillRect(84, 547, countBar(max, cc.health), 4);
    	g.setColor(Color.BLUE);
    	g.fillRect(84, 562, countBar(max, cc.energy), 4);
    	g.setColor(Color.GREEN);
    	g.fillRect(84, 577, countBar(max, cc.strength), 4);
    	g.fillRect(84, 592, countBar(max, cc.speed), 4);
    	g.fillRect(84, 607, countBar(max, cc.intelligence), 4);
    	g.fillRect(84, 622, countBar(max, cc.armor), 4);
    	
    	g.drawImage(chatGsm.getImage(),630,541,this);
    	g.drawImage(chatTeam.getImage(),630,564,this);
    	g.drawImage(chatYell.getImage(),630,587,this);
    	g.drawImage(chatAll.getImage(),630,610,this);

    	// Draw the status picture.
		int st_x = 15, st_y = 562; // x and y for status picture. 
		if (gv.players[gv.playerID].monster)
			g.drawImage(st_monster.getImage(),st_x,st_y,this);
		else if (cc.playerType == Constants.DWARF)
			g.drawImage(st_dwarf.getImage(),st_x,st_y,this);
		else if (cc.playerType == Constants.WARRIOR)
			g.drawImage(st_warrior.getImage(),st_x,st_y,this);
		else if (cc.playerType == Constants.ADVENTURER)
			g.drawImage(st_adventurer.getImage(),st_x,st_y,this);
		else
			g.drawImage(st_elf.getImage(),st_x,st_y,this);

		// Draw the attributes in the interface.
    	int ax, ay;
    	for (int i = 0; i < Constants.MAX_ATTRIBUTES_CARRIED; i++) {
    	    if (i <= 2) {
    			ax = 700 + (i * 46);
    			ay = 549;
    		} else { 
    			ax = 700 + ((i - 3) * 46);
    			ay = 593;
    		}
    	    ViewUseAttribute va = cc.attribute[i];
    	    if (va != null) {
    	        if (va.active) {
	    	        switch (va.type) {
	    	        case Constants.ATTR_ARMOR:
	    	        	g.drawImage(armor_active.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_BROOMSTICK:
	    	        	g.drawImage(broomstick_active.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_INVISIBILITY:
	    	        	g.drawImage(invisibility_active.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_LIPSTICK:
	    	        	g.drawImage(lipstick_active.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_NIGHTVISION:
	    	        	g.drawImage(nightvision_active.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_SWORD:
	    	        	g.drawImage(sword_active.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_SHIELD:
	    	        	g.drawImage(shield_active.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_VITAMINES:
	    	        	g.drawImage(vitamines.getImage(),ax,ay,this);
		        		break;
	    	        }
    	        } else {
	    	        switch (va.type) {
	    	        case Constants.ATTR_ARMOR:
	    	        	g.drawImage(armor.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_BROOMSTICK:
	    	        	g.drawImage(broomstick.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_INVISIBILITY:
	    	        	g.drawImage(invisibility.getImage(),ax,ay,this);
	    	        	break;
	    	        case Constants.ATTR_LIPSTICK:
	    	        	g.drawImage(lipstick.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_NIGHTVISION:
	    	        	g.drawImage(nightvision.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_SWORD:
	    	        	g.drawImage(sword.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_SHIELD:
	    	        	g.drawImage(shield.getImage(),ax,ay,this);
		        		break;
	    	        case Constants.ATTR_VITAMINES:
	    	        	g.drawImage(vitamines.getImage(),ax,ay,this);
		        		break;
	    	        }
    	        }
    	    }
    	}
	}

	private int countBar(int max, int value)
	{
		return (value * 119) / max;
	}

	/**
	 * Update the attributes in the view.
	 */
	public void updateViewAttributes()
	{
		repaint();
	}
	
	public void teamChat(String sMessage, String sPlayerName)
	{
		chatOutput.append("(Team)" + sPlayerName + ": " + sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}
	
	public void yell(String sMessage, String sPlayerName)
	{
		chatOutput.append("(Yell)" + sPlayerName + ": " + sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}

	public void chat(String sMessage, String sPlayerName)
	{
		chatOutput.append("(Chat)" + sPlayerName + ": " + sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}
	
	public void gsm(String sMessage, String sPlayerName)
	{
		chatOutput.append("(GSM)" + sPlayerName + ": " + sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}

	/**
	 * Sends a global msg.
	 * 
	 * @param sMessage
	 */
	public void gameEvent(String sMessage)
	{
		chatOutput.append(sMessage + "\n");
		chatOutput.setCaretPosition(chatOutput.getText().length());
	}

	public void actionPerformed(ActionEvent e)
	{
		if (e.getSource() == btnChatWhisper) {
			this.transferFocusUpCycle();
			this.gsm = new GSMFrame(g, gv);			
		} else if (!chatInput.getText().equals("")) {
			if (e.getSource() == btnChatTeam) {
				try {
					g.action_SendMessage(Constants.ACTION_TEAMCHAT, gv.playerID, gv.activeCharacter, chatInput.getText());
				} catch (RemoteException e1) {
				    System.err.println("RemoteException in action_SendMessage: " + e1.getMessage());
				}
				chatInput.setText("");
			} else if (e.getSource() == btnChatYell) {
				try {
					g.action_SendMessage(Constants.ACTION_YELL, gv.playerID, gv.activeCharacter, chatInput.getText());
				} catch (RemoteException e1) {
				    System.err.println("RemoteException in action_SendMessage: " + e1.getMessage());
				}
				chatInput.setText("");
			} else if (e.getSource() == btnChatAll) {
				try {
					g.action_SendMessage(Constants.ACTION_CHAT, gv.playerID, gv.activeCharacter, chatInput.getText());
				} catch (RemoteException e1) {
				    System.err.println("RemoteException in action_SendMessage: " + e1.getMessage());
				}
				chatInput.setText("");
			}
		}
	}

	/**
	 * Retrieve the GSM frame.
	 * 
	 * @return GSMFrame
	 */
	public GSMFrame getGSM()
	{
		return this.gsm;
	}

	public void keyPressed(KeyEvent arg0)
	{
		if (arg0.getKeyCode() == KeyEvent.VK_ENTER && !chatInput.getText().equals("")) {
			try {
                g.action_SendMessage(Constants.ACTION_CHAT, gv.playerID, gv.activeCharacter, chatInput.getText());
            } catch (RemoteException e) {
                System.err.println("RemoteException in action_Chat: " + e.getMessage());
            }
			chatInput.setText("");
		}
	}

	public void keyReleased(KeyEvent arg0)
	{
	}
	
	public void keyTyped(KeyEvent arg0)
	{
	}
}
