package nl.pointless.webmail.web;

import javax.servlet.http.HttpServletResponse;

import nl.pointless.webmail.web.component.WebmailPage;

import org.apache.wicket.Page;
import org.apache.wicket.Request;
import org.apache.wicket.Response;
import org.apache.wicket.Session;
import org.apache.wicket.protocol.http.HttpSessionStore;
import org.apache.wicket.protocol.http.WebApplication;
import org.apache.wicket.protocol.http.WebResponse;
import org.apache.wicket.session.ISessionStore;
import org.apache.wicket.spring.injection.annot.SpringComponentInjector;
import org.apache.wicket.util.tester.WicketTester;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * WicketTester for the webmail application.
 * 
 * @author Peter Postma
 */
public class WebmailTester extends WicketTester {

	/**
	 * Mock web application.
	 * 
	 * @author Peter Postma
	 */
	private static class WebmailMockWebApplication extends WebApplication {

		public WebmailMockWebApplication() {
			super();
		}

		@Override
		public Class<? extends Page> getHomePage() {
			return WebmailPage.class;
		}

		@Override
		protected ISessionStore newSessionStore() {
			return new HttpSessionStore(this);
		}

		@Override
		protected WebResponse newWebResponse(
				final HttpServletResponse servletResponse) {
			return new WebResponse(servletResponse);
		}

		@Override
		public Session newSession(Request request, Response response) {
			return new WebmailSession(request);
		}

		@Override
		protected void outputDevelopmentModeWarning() {
			// do nothing
		}
	}

	public WebmailTester() {
		super(new WebmailMockWebApplication());

		ApplicationContext context = new ClassPathXmlApplicationContext(
				new String[] { "mockContext.xml" });

		getApplication().addComponentInstantiationListener(
				new SpringComponentInjector(getApplication(), context, true));
	}
}
