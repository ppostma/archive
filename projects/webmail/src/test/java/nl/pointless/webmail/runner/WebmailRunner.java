package nl.pointless.webmail.runner;

import org.mortbay.jetty.Connector;
import org.mortbay.jetty.Server;
import org.mortbay.jetty.bio.SocketConnector;
import org.mortbay.jetty.webapp.WebAppContext;

/**
 * Runner for the webmail application.
 * 
 * @author Peter Postma
 */
public class WebmailRunner {

	public static void main(String[] args) throws Exception {
		System.setProperty("wicket.configuration", "development");

		SocketConnector connector = new SocketConnector();
		connector.setMaxIdleTime(1000 * 60 * 60);
		connector.setSoLingerTime(-1);
		connector.setPort(8080);

		Server server = new Server();
		server.setConnectors(new Connector[] { connector });

		WebAppContext bb = new WebAppContext();
		bb.setServer(server);
		bb.setContextPath("/webmail");
		bb.setWar("src/main/webapp");

		server.addHandler(bb);

		try {
			server.start();
		} catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
	}
}
