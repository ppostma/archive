package nl.pointless.commons.web.component;

import org.apache.wicket.RequestCycle;
import org.apache.wicket.ResourceReference;
import org.apache.wicket.ajax.IAjaxIndicatorAware;
import org.apache.wicket.ajax.markup.html.AjaxLink;
import org.apache.wicket.extensions.ajax.markup.html.AjaxIndicatorAppender;
import org.apache.wicket.model.IModel;

/**
 * An {@link AjaxLink} that adds a busy indicator when the link is being loaded.
 * 
 * @author Peter Postma
 * @param T Type of the link.
 */
public abstract class AbstractIndicatingAjaxLink<T> extends AjaxLink<T>
		implements IAjaxIndicatorAware {

	private static final long serialVersionUID = 1L;

	private static final ResourceReference AJAX_LOADER = new ResourceReference(
			AbstractIndicatingAjaxLink.class, "images/ajax-loader.gif");

	private final AjaxIndicatorAppender indicatorAppender = new AjaxIndicatorAppender() {

		private static final long serialVersionUID = 1L;

		/**
		 * {@inheritDoc}
		 */
		protected CharSequence getIndicatorUrl() {
			return RequestCycle.get().urlFor(AJAX_LOADER);
		}
	};

	/**
	 * Constructor.
	 * 
	 * @param id Component id.
	 */
	public AbstractIndicatingAjaxLink(String id) {
		this(id, null);
	}

	/**
	 * Constructor.
	 * 
	 * @param id Component id.
	 * @param model Model.
	 */
	public AbstractIndicatingAjaxLink(String id, IModel<T> model) {
		super(id, model);
		add(this.indicatorAppender);
	}

	/**
	 * {@inheritDoc}
	 */
	public String getAjaxIndicatorMarkupId() {
		return this.indicatorAppender.getMarkupId();
	}
}
