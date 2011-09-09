package nl.pointless.commons.web.behavior;

import org.apache.wicket.Component;
import org.apache.wicket.behavior.Behavior;
import org.apache.wicket.markup.html.IHeaderResponse;

/**
 * Set the focus on the component when loading the page.
 * 
 * @author Peter Postma
 */
public class FocusBehavior extends Behavior {

	private static final long serialVersionUID = 1L;

	private Component component;

	@Override
	public void bind(Component component) {
		this.component = component;
		this.component.setOutputMarkupId(true);
	}

	@Override
	public void renderHead(Component component, IHeaderResponse headerResponse) {
		headerResponse.renderOnLoadJavaScript("document.getElementById('"
				+ this.component.getMarkupId() + "').focus();");
	}
}
