#ifndef CHAOSLABEL_H
#define CHAOSLABEL_H
#include "../monitor/handler/handler.h"

#include <QLabel>
#include <chaos/common/chaos_constants.h>

#include <ChaosMetadataServiceClient/ChaosMetadataServiceClient.h>

#include <boost/shared_ptr.hpp>

typedef chaos::DataType::DataType ChaosDataType;

class ChaosLabel:
        public QLabel {
    Q_OBJECT

    unsigned int p_timeout_for_alive;
    Q_PROPERTY(unsigned int timeout_for_alive READ timeoutForAlive WRITE setTimeoutForAlive)

    QString p_node_uid;
    Q_PROPERTY(QString node_uid READ nodeUniqueID WRITE setNodeUniqueID NOTIFY nodeUniqueIDChanged)

    QString p_attribute_name;
    Q_PROPERTY(QString attribute_name READ attributeName WRITE setAttributeName NOTIFY attributeNameChanged)

    ChaosDataType p_attribute_type;
    Q_PROPERTY(ChaosDataType attribute_type READ attributeType WRITE setAttributeType NOTIFY attributeTypeChanged)
    Q_ENUMS(ChaosDataType)

    bool p_track_status;
    Q_PROPERTY(bool track_status READ trackStatus WRITE setTrackStatus)

    bool p_label_value_show_track_status;
    Q_PROPERTY(bool label_value_show_track_status READ labelValueShowTrackStatus WRITE setLabelValueShowTrackStatus)

    QVariant current_value;
public:
    ChaosLabel(QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~ChaosLabel();

    void setNodeUniqueID(const QString& node_uid);
    QString nodeUniqueID();

    void setAttributeName(const QString& attribute_name);
    QString attributeName();

    void setAttributeType(ChaosDataType attribute_type);
    ChaosDataType attributeType();

    void setTimeoutForAlive(unsigned int timeout_for_alive);
    unsigned int timeoutForAlive();

    void setTrackStatus(bool track_status);
    bool trackStatus();

    virtual void setLabelValueShowTrackStatus(bool label_value_show_track_status);
    bool labelValueShowTrackStatus();

    virtual int startMonitoring();

    virtual int stopMonitoring();

    void	setText(const QString &string);
protected slots:
    virtual void valueUpdated(const QString& node_uid,
                              const QString& attribute_name,
                              uint64_t timestamp,
                              const QVariant& attribute_value);
    virtual void valueUpdated(const QString& node_uid,
                              const QString& attribute_name,
                              const QVariant& attribute_value);
    virtual void valueNotFound(const QString& node_uid,
                              const QString& attribute_name);

signals:
    void nodeUniqueIDChanged(const QString& last_node_uid,
                             const QString& new_node_uid);
    void attributeNameChanged(const QString& node_uid,
                              const QString& last_attribute_name,
                              const QString& new_attribute_name);
    void attributeTypeChanged(ChaosDataType last_type,
                              ChaosDataType new_type);
    void valueChanged(const QString& node_uid,
                      const QString& value);
protected:
    bool monitoring;
    uint64_t last_recevied_ts;
    uint32_t zero_diff_count;
    HealthHeartbeatHandler healt_heartbeat_handler;
    HealtStatusHandler healt_status_handler;
    boost::shared_ptr<AbstractTSTaggedAttributeHandler> handler_sptr;
    //hide public default slot
    void	clear();
    void	setMovie(QMovie * movie);
    void	setNum(int num);
    void	setNum(double num);
    void	setPicture(const QPicture & picture);
    void	setPixmap(const QPixmap &pixmap);

};

#endif // CHAOSLABEL_H
