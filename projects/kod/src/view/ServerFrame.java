/* $Id: ServerFrame.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * ServerFrame.java
 * 
 * The server frame.
 */

package src.view;

import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.io.PrintStream;
import java.rmi.ConnectException;
import java.rmi.Naming;
import java.rmi.RMISecurityManager;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.sql.Time;
import java.util.Date;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;

import src.controller.Game;
import src.persistence.MapReader;
import src.utils.Constants;

public class ServerFrame extends JFrame implements ActionListener, Runnable
{
	private JPanel contentPane;
	private JLabel labelMap = new JLabel();
	private JComboBox comboMap = new JComboBox();
	private JButton button = new JButton();
	private JTextArea textMessages = new JTextArea();
	private PipedInputStream pin = new PipedInputStream();
	private PipedInputStream pin2 = new PipedInputStream();
	private Thread reader, reader2;

	private boolean started;

	public ServerFrame()
	{
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setTitle("Kiss of Death :: Server");

		contentPane = (JPanel)getContentPane();
	    contentPane.setVisible(true);
	    contentPane.setLayout(null);

	    labelMap.setBounds(new Rectangle(40, 30, 40, 20));
	    labelMap.setText("Map:");

	    comboMap.setBounds(new Rectangle(80, 30, 150, 20));
	    String[] maps = MapReader.getMapNames(Constants.DIRECTORY_MAPS_BASE);
	    for (int i = 0; i < maps.length; i++)
	        comboMap.addItem(maps[i]);

	    button.setBounds(new Rectangle(260, 30, 80, 20));
	    button.setText("Start");
	    button.addActionListener(this);

	    textMessages.setEditable(false);
	    textMessages.setLineWrap(true);
	    textMessages.setWrapStyleWord(true);
	    textMessages.setVisible(true);
		JScrollPane scrollMessages = 
			new JScrollPane(textMessages,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		scrollMessages.setBounds(new Rectangle(10, 65, 475, 205));

		contentPane.add(labelMap);
		contentPane.add(button);
		contentPane.add(comboMap);
		contentPane.add(scrollMessages);

	    setResizable(false);
	    setSize(500, 300);
	    setVisible(true);

	    try {
	    	PipedOutputStream pout = new PipedOutputStream(this.pin);
	    	System.setOut(new PrintStream(pout, true));
		} catch (IOException e) {
			consoleLog("Can't redirect stdout to this console.\n");
		}

		try {
			PipedOutputStream pout = new PipedOutputStream(this.pin2);
			System.setErr(new PrintStream(pout, true));
		} catch (IOException e) {
			consoleLog("Can't redirect stderr to this console.\n");
		}

		reader = new Thread(this);
		reader.setDaemon(true);
		reader.start();

		reader2 = new Thread(this);
		reader2.setDaemon(true);
		reader2.start();

		started = false;
	}

	public void actionPerformed(ActionEvent arg)
	{
		if (arg.getSource() == button && !started) {

		    if (comboMap.getSelectedItem() == null) {
		        consoleLog("Please select a map.\n");
		        return;
		    }

		    button.setEnabled(false);
		    try {
		        String map = Constants.DIRECTORY_MAPS_BASE + comboMap.getSelectedItem();
				Naming.rebind(Constants.GAME_NAME, new Game(map));
		    } catch (Exception e) {	
		        consoleLog("Unable to start the server: " + e.getMessage() + "\n");
		        if (e instanceof ConnectException)
		            consoleLog("Is the rmiregistry service running?\n");
		        button.setEnabled(true);
				return;
			}

		    consoleLog("Server is ready.\n");
		    this.started = true;

		    button.setText("Stop");
		    button.setEnabled(true);
		} else if (arg.getSource() == button && started) {

		    button.setEnabled(false);

		    try {
                Naming.unbind(Constants.GAME_NAME);
            } catch (Exception e) {
		        consoleLog("Unable to stop the server: " + e.getMessage() + "\n");
		        button.setEnabled(true);
				return;
            }

            consoleLog("Server has been stopped.\n");
            this.started = false;

            button.setText("Start");
		    button.setEnabled(true);
		}
	}

	private synchronized void consoleLog(String message)
	{
		Time t = new Time(new Date().getTime());
		textMessages.append("[" + t.toString() + "] " + message);
		textMessages.setCaretPosition(textMessages.getText().length());
	}

	private synchronized String readLine(PipedInputStream in) throws IOException
	{
		String input = "";

		do {
			if (in.available() == 0)
				break;
			input = input + (char)in.read();
		} while (!input.endsWith("\n") && !input.endsWith("\r\n"));

		return input;
	}

	public void run()
	{
		/* Stdout redirect thread. */
		while (Thread.currentThread() == reader) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) { }

			try {
				while (pin.available() != 0) {
					String input = this.readLine(pin);
					consoleLog(input);
				}
			} catch (IOException e) {
				consoleLog("Internal console error: " + e.getMessage() + "\n");
			}
		}

		/* Stderr redirect thread. */
		while (Thread.currentThread() == reader2) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) { }

			try {
				while (pin2.available() != 0) {
					String input = this.readLine(pin2);
					consoleLog("ERROR: " + input);
				}
			} catch (IOException e) {
				consoleLog("Internal console error: " + e.getMessage() + "\n");
			}
		}
	}

	public static void main(String[] args)
	{
	    if (System.getProperty("java.server.rmi.codebase") == null)
	        System.setProperty("java.server.rmi.codebase", System.getProperty("java.class.path"));

	    if (System.getProperty("java.security.policy") == null)
	        System.setProperty("java.security.policy", ".java.policy");

       	try {
    		LocateRegistry.createRegistry(1099);
    	} catch (RemoteException e) {
    		System.err.println("createRegistry exception: " + e.getMessage());
    		System.exit(1);
    	}

	    if (System.getSecurityManager() == null)
            System.setSecurityManager(new RMISecurityManager());

		new ServerFrame();
	}
}
