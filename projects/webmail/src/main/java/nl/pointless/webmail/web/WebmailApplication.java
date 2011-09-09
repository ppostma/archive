package nl.pointless.webmail.web;

import nl.pointless.webmail.web.component.LoginPage;
import nl.pointless.webmail.web.component.WebmailPage;

import org.apache.wicket.Session;
import org.apache.wicket.authorization.strategies.page.SimplePageAuthorizationStrategy;
import org.apache.wicket.protocol.http.WebApplication;
import org.apache.wicket.request.Request;
import org.apache.wicket.request.Response;
import org.apache.wicket.spring.injection.annot.SpringComponentInjector;

/**
 * Application object for the webmail application.
 * 
 * @author Peter Postma
 */
public class WebmailApplication extends WebApplication {

	@Override
	protected void init() {
		getApplicationSettings().setPageExpiredErrorPage(LoginPage.class);

		getMarkupSettings().setDefaultMarkupEncoding("UTF-8");
		getMarkupSettings().setStripWicketTags(true);

		SimplePageAuthorizationStrategy authorizationStrategy = new SimplePageAuthorizationStrategy(
				WebmailPage.class, LoginPage.class) {

			@Override
			protected boolean isAuthorized() {
				return WebmailSession.get().isAuthenticated();
			}
		};
		getSecuritySettings().setAuthorizationStrategy(authorizationStrategy);

		getComponentInstantiationListeners().add(
				new SpringComponentInjector(this));

		mountPage("/login", LoginPage.class);
	}

	@Override
	public Session newSession(Request request, Response response) {
		return new WebmailSession(request);
	}

	@Override
	public Class<WebmailPage> getHomePage() {
		return WebmailPage.class;
	}
}
