package nl.pointless.commons.web.component;

import org.apache.wicket.markup.html.form.TextField;
import org.apache.wicket.model.IModel;

/**
 * A {@link TextField} for HTML5 &lt;input&gt; with type <em>search</em>.
 * 
 * @author Peter Postma
 */
public class SearchTextField extends TextField<String> {

	private static final long serialVersionUID = 1L;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket id.
	 */
	public SearchTextField(String id) {
		super(id);
	}

	/**
	 * Constructor.
	 * 
	 * @param id Wicket id.
	 * @param model Model.
	 */
	public SearchTextField(String id, IModel<String> model) {
		super(id, model);
	}

	@Override
	protected String getInputType() {
		return "search";
	}
}
