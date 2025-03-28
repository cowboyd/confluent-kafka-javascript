/*
 * confluent-kafka-javascript - Node.js wrapper  for RdKafka C/C++ library
 * Copyright (c) 2016-2023 Blizzard Entertainment
 *           (c) 2023 Confluent, Inc.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE.txt file for details.
 */

#ifndef SRC_CALLBACKS_H_
#define SRC_CALLBACKS_H_

#include <uv.h>
#include <napi.h>
#include <uv.h>

#include <vector>
#include <deque>

#include "rdkafkacpp.h" // NOLINT
#include "src/common.h"

namespace NodeKafka {

class KafkaConsumer;

namespace Callbacks {

class Dispatcher {
 public:
  Dispatcher();
  ~Dispatcher();
  void Dispatch(const int, Napi::Value []);
  void AddCallback(const Napi::Function&);
  void RemoveCallback(const Napi::Function&);
  bool HasCallbacks();
  virtual void Flush() = 0;
  void Execute();
  void Activate();
  void Deactivate();

 protected:
  std::vector<Napi::FunctionReference*> callbacks;  // NOLINT

  uv_mutex_t async_lock;

 private:
  inline static void func(uv_async_t *async) {
     Dispatcher *dispatcher =
	    static_cast<Dispatcher*>(async->data);
     dispatcher->Flush();
  }
  static void AsyncHandleCloseCallback(uv_handle_t *);

  uv_async_t *async;
};

struct event_t {
  RdKafka::Event::Type type;
  std::string message;

  RdKafka::Event::Severity severity;
  std::string fac;

  std::string broker_name;
  int throttle_time;
  int broker_id;

  explicit event_t(const RdKafka::Event &);
  ~event_t();
};

class EventDispatcher : public Dispatcher {
 public:
  EventDispatcher();
  ~EventDispatcher();
  void Add(const event_t &);
  void Flush();
  void SetClientName(const std::string &);
 protected:
  std::vector<event_t> events;
  std::string client_name;
};

class Event : public RdKafka::EventCb {
 public:
  Event();
  ~Event();
  void event_cb(RdKafka::Event&);
  EventDispatcher dispatcher;
};

/**
 * Delivery report class
 *
 * Class exists because the callback needs to be able to give information
 * to the v8 main thread that it can use to formulate its object.
 */
class DeliveryReport {
 public:
  DeliveryReport(RdKafka::Message &, bool);
  ~DeliveryReport();

  // Whether we include the payload. Is the second parameter to the constructor
  bool m_include_payload;

  // If it is an error these will be set
  bool is_error;
  std::string error_string;
  RdKafka::ErrorCode error_code;

  // If it is not
  std::string topic_name;
  int32_t partition;
  int64_t offset;
  int64_t timestamp;

  // Opaque token used. Local value
  void* opaque;

  // Key. It is a pointer to avoid corrupted values
  // https://github.com/confluentinc/confluent-kafka-javascript/issues/208
  void* key;
  size_t key_len;

  size_t len;
  void* payload;
};

class DeliveryReportDispatcher : public Dispatcher {
 public:
  DeliveryReportDispatcher();
  ~DeliveryReportDispatcher();
  void Flush();
  size_t Add(const DeliveryReport &);
 protected:
  std::deque<DeliveryReport> events;
};

class Delivery : public RdKafka::DeliveryReportCb {
 public:
  Delivery();
  ~Delivery();
  void dr_cb(RdKafka::Message&);
  DeliveryReportDispatcher dispatcher;
  void SendMessageBuffer(bool dr_copy_payload);
 protected:
  bool m_dr_msg_cb;
};

// Rebalance dispatcher

struct event_topic_partition_t {
  std::string topic;
  int partition;
  int64_t offset;

  event_topic_partition_t(std::string p_topic, int p_partition, int64_t p_offset):  // NOLINT
    topic(p_topic),
    partition(p_partition),
    offset(p_offset) {}
};

struct rebalance_event_t {
  RdKafka::ErrorCode err;
  std::vector<event_topic_partition_t> partitions;

  rebalance_event_t(RdKafka::ErrorCode p_err,
	std::vector<RdKafka::TopicPartition*> p_partitions):
	err(p_err) {
    // Iterate over the topic partitions because we won't have them later
    for (size_t topic_partition_i = 0;
      topic_partition_i < p_partitions.size(); topic_partition_i++) {
      RdKafka::TopicPartition* topic_partition =
	p_partitions[topic_partition_i];

      event_topic_partition_t tp(
	topic_partition->topic(),
	topic_partition->partition(),
	topic_partition->offset());

      partitions.push_back(tp);
    }
  }
};

struct offset_commit_event_t {
  RdKafka::ErrorCode err;
  std::vector<event_topic_partition_t> partitions;

  offset_commit_event_t(RdKafka::ErrorCode p_err,
    const std::vector<RdKafka::TopicPartition*> &p_partitions):
    err(p_err) {
    // Iterate over the topic partitions because we won't have them later
    for (size_t topic_partition_i = 0;
      topic_partition_i < p_partitions.size(); topic_partition_i++) {
      RdKafka::TopicPartition* topic_partition =
	p_partitions[topic_partition_i];

      // Just reuse this thing because it's the same exact thing we need
      event_topic_partition_t tp(
	topic_partition->topic(),
	topic_partition->partition(),
	topic_partition->offset());

      partitions.push_back(tp);
    }
  }
};

class RebalanceDispatcher : public Dispatcher {
 public:
  RebalanceDispatcher();
  ~RebalanceDispatcher();
  void Add(const rebalance_event_t &);
  void Flush();
 protected:
  std::vector<rebalance_event_t> m_events;
};

class Rebalance : public RdKafka::RebalanceCb {
 public:
  void rebalance_cb(RdKafka::KafkaConsumer *, RdKafka::ErrorCode,
    std::vector<RdKafka::TopicPartition*> &);

  RebalanceDispatcher dispatcher;
 // private:
 //  v8::Persistent<v8::Function> m_cb;
};

class OffsetCommitDispatcher : public Dispatcher {
 public:
  OffsetCommitDispatcher();
  ~OffsetCommitDispatcher();
  void Add(const offset_commit_event_t &);
  void Flush();
 protected:
  std::vector<offset_commit_event_t> m_events;
};

class OffsetCommit : public RdKafka::OffsetCommitCb {
 public:
  void offset_commit_cb(RdKafka::ErrorCode, std::vector<RdKafka::TopicPartition*> &);  // NOLINT

  OffsetCommitDispatcher dispatcher;
 // private:
 //  v8::Persistent<v8::Function> m_cb;
};

class OAuthBearerTokenRefreshDispatcher : public Dispatcher {
 public:
  OAuthBearerTokenRefreshDispatcher() {}
  ~OAuthBearerTokenRefreshDispatcher() {}
  void Add(const std::string &oauthbearer_config);
  void Flush();

 private:
  std::string m_oauthbearer_config;
};

class OAuthBearerTokenRefresh : public RdKafka::OAuthBearerTokenRefreshCb {
 public:
  void oauthbearer_token_refresh_cb(RdKafka::Handle *, const std::string &);
  OAuthBearerTokenRefreshDispatcher dispatcher;
};

class Partitioner : public RdKafka::PartitionerCb {
 public:
  Partitioner();
  ~Partitioner();
  int32_t partitioner_cb( const RdKafka::Topic*, const std::string*, int32_t, void*);  // NOLINT
  Napi::FunctionReference callback;  // NOLINT
  void SetCallback(Napi::Function);
 private:
  static unsigned int djb_hash(const char*, size_t);
  static unsigned int random(const RdKafka::Topic*, int32_t);
};

class QueueNotEmptyDispatcher : public Dispatcher {
 public:
  QueueNotEmptyDispatcher();
  ~QueueNotEmptyDispatcher();
  void Flush();
};

// This callback does not extend from any class because it's a C callback
// The only reason we have a class here is to maintain similarity with how
// callbacks are structured in the rest of the library.
class QueueNotEmpty {
 public:
  QueueNotEmpty();
  ~QueueNotEmpty();
  // This is static because it must be passed to C. The `this` reference will be
  // passde within `self`.
  static void queue_not_empty_cb(rd_kafka_t *rk, void *self);
  QueueNotEmptyDispatcher dispatcher;
};

}  // namespace Callbacks

}  // namespace NodeKafka

#endif  // SRC_CALLBACKS_H_
