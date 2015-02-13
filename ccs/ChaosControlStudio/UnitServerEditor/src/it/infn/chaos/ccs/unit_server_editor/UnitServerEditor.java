/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package it.infn.chaos.ccs.unit_server_editor;

import it.infn.chaos.ccs.low_api.event.ShowPropertyEvent;
import it.infn.chaos.ccs.low_api.node.UnitServerNode;
import java.util.Collections;
import org.openide.awt.ActionID;
import org.openide.awt.ActionReference;
import org.openide.util.NbBundle;
import org.openide.util.lookup.AbstractLookup;
import org.openide.util.lookup.InstanceContent;
import org.openide.windows.TopComponent;

@TopComponent.Description(
        preferredID = "UnitServerEditor",
        persistenceType = TopComponent.PERSISTENCE_ALWAYS)
@TopComponent.Registration(
        mode = "editor",
        openAtStartup = false)
@ActionID(
        category = "Window",
        id = "it.infn.chaos.css.unit_server_editor.UnitServerEditor")
@ActionReference(
        path = "Menu/Window")
@NbBundle.Messages({
    "CTL_UnitServerEditorAction=UnitServerEditor"
})

/**
 *
 * @author bisegni
 */
public class UnitServerEditor extends TopComponent {

    private final UnitServerNode us;
    private final InstanceContent content = new InstanceContent();

    /**
     * Creates new form UnitServerEditor
     *
     * @param us
     */
    public UnitServerEditor(UnitServerNode us) {
        initComponents();
        setName(Bundle.CTL_UnitServerEditorAction()+"/"+us.getNodeUniqueID());
        setToolTipText(Bundle.CTL_UnitServerEditorAction());
        this.us = us;
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jLabel1 = new javax.swing.JLabel();
        jTextFieldUniqueID = new javax.swing.JTextField();
        jButton1 = new javax.swing.JButton();

        org.openide.awt.Mnemonics.setLocalizedText(jLabel1, org.openide.util.NbBundle.getMessage(UnitServerEditor.class, "UnitServerEditor.jLabel1.text")); // NOI18N

        jTextFieldUniqueID.setText(org.openide.util.NbBundle.getMessage(UnitServerEditor.class, "UnitServerEditor.jTextFieldUniqueID.text")); // NOI18N
        jTextFieldUniqueID.setMaximumSize(new java.awt.Dimension(350, 2147483647));

        org.openide.awt.Mnemonics.setLocalizedText(jButton1, org.openide.util.NbBundle.getMessage(UnitServerEditor.class, "UnitServerEditor.jButton1.text")); // NOI18N

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jButton1, javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jTextFieldUniqueID, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 235, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel1)
                    .addComponent(jTextFieldUniqueID, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jButton1)
                .addContainerGap(195, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButton1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JTextField jTextFieldUniqueID;
    // End of variables declaration//GEN-END:variables
 @Override
    protected void componentDeactivated() {
        System.out.println("componentDeactivated");
    }

    @Override
    protected void componentActivated() {
        System.out.println("componentActivated");
        ShowPropertyEvent spe = new ShowPropertyEvent(us);
        content.set(Collections.singleton(spe), null);
    }

    @Override
    protected void componentShowing() {
        System.out.println("componentShowing");
    }

    @Override
    protected void componentClosed() {
        System.out.println("componentClosed");
    }

    @Override
    protected void componentOpened() {
        System.out.println("componentOpened");
        associateLookup(new AbstractLookup(content));
        jTextFieldUniqueID.setText(us.getNodeUniqueID());
    }
}
