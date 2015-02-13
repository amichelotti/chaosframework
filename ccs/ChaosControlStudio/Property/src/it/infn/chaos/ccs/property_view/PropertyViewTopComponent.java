/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package it.infn.chaos.ccs.property_view;

import it.infn.chaos.ccs.low_api.event.AbstractEvent;
import it.infn.chaos.ccs.low_api.event.ShowPropertyEvent;
import java.beans.IntrospectionException;
import java.util.Collection;
import org.netbeans.api.settings.ConvertAsProperties;
import org.openide.awt.ActionID;
import org.openide.awt.ActionReference;
import org.openide.nodes.BeanNode;
import org.openide.nodes.Node;
import org.openide.util.Exceptions;
import org.openide.util.Lookup;
import org.openide.util.LookupEvent;
import org.openide.util.LookupListener;
import org.openide.util.NbBundle.Messages;
import org.openide.util.Utilities;
import org.openide.windows.TopComponent;

/**
 * Top component which displays something.
 */
@ConvertAsProperties(
        dtd = "-//it.infn.chaos.css.property_view//PropertyView//EN",
        autostore = false
)
@TopComponent.Description(
        preferredID = "PropertyViewTopComponent",
        //iconBase="SET/PATH/TO/ICON/HERE", 
        persistenceType = TopComponent.PERSISTENCE_ALWAYS
)
@TopComponent.Registration(mode = "properties", openAtStartup = true)
@ActionID(category = "Window", id = "it.infn.chaos.css.property_view.PropertyViewTopComponent")
@ActionReference(path = "Menu/Window" /*, position = 333 */)
@TopComponent.OpenActionRegistration(
        displayName = "#CTL_PropertyViewAction",
        preferredID = "PropertyViewTopComponent"
)
@Messages({
    "CTL_PropertyViewAction=PropertyView",
    "CTL_PropertyViewTopComponent=PropertyView Window",
    "HINT_PropertyViewTopComponent=This is a PropertyView window"
})
public final class PropertyViewTopComponent extends TopComponent implements LookupListener {

    private Lookup.Result<ShowPropertyEvent> result;

    public PropertyViewTopComponent() {
        initComponents();
        setName(Bundle.CTL_PropertyViewTopComponent());
        setToolTipText(Bundle.HINT_PropertyViewTopComponent());

    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        propertySheetView1 = new org.openide.explorer.propertysheet.PropertySheetView();

        setLayout(new java.awt.BorderLayout());
        add(propertySheetView1, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private org.openide.explorer.propertysheet.PropertySheetView propertySheetView1;
    // End of variables declaration//GEN-END:variables
    @Override
    public void componentOpened() {
        result = Utilities.actionsGlobalContext().lookupResult(ShowPropertyEvent.class);
        result.allItems();
        result.addLookupListener(this);
    }

    @Override
    public void componentClosed() {
        result.removeLookupListener(this);
    }

    void writeProperties(java.util.Properties p) {
        // better to version settings since initial version as advocated at
        // http://wiki.apidesign.org/wiki/PropertyFiles
        p.setProperty("version", "1.0");
        // TODO store your settings
    }

    void readProperties(java.util.Properties p) {
        String version = p.getProperty("version");
        // TODO read your settings according to their version
    }

    @Override
    public void resultChanged(LookupEvent le) {
        Collection<? extends AbstractEvent> allEvents = result.allInstances();
        if (!allEvents.isEmpty()) {
            AbstractEvent nodeEvent = allEvents.iterator().next();
            try {
               
                propertySheetView1.setNodes(new Node[]{new BeanNode(nodeEvent.getEventTarget())});
            } catch (IntrospectionException ex) {
                Exceptions.printStackTrace(ex);
            }
        }
    }
}
