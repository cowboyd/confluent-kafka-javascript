// @generated by protoc-gen-es v2.2.3 with parameter "target=ts"
// @generated from file test/schemaregistry/serde/dep.proto (package test, syntax proto3)
/* eslint-disable */

import type { GenFile, GenMessage } from "@bufbuild/protobuf/codegenv1";
import { fileDesc, messageDesc } from "@bufbuild/protobuf/codegenv1";
import type { TestMessage } from "./test_pb";
import { file_test_schemaregistry_serde_test } from "./test_pb";
import type { Message } from "@bufbuild/protobuf";

/**
 * Describes the file test/schemaregistry/serde/dep.proto.
 */
export const file_test_schemaregistry_serde_dep: GenFile = /*@__PURE__*/
  fileDesc("CiN0ZXN0L3NjaGVtYXJlZ2lzdHJ5L3NlcmRlL2RlcC5wcm90bxIEdGVzdCJQChFEZXBlbmRlbmN5TWVzc2FnZRIRCglpc19hY3RpdmUYASABKAgSKAoNdGVzdF9tZXNzc2FnZRgCIAEoCzIRLnRlc3QuVGVzdE1lc3NhZ2VCCVoHLi4vdGVzdGIGcHJvdG8z", [file_test_schemaregistry_serde_test]);

/**
 * @generated from message test.DependencyMessage
 */
export type DependencyMessage = Message<"test.DependencyMessage"> & {
  /**
   * @generated from field: bool is_active = 1;
   */
  isActive: boolean;

  /**
   * @generated from field: test.TestMessage test_messsage = 2;
   */
  testMesssage?: TestMessage;
};

/**
 * Describes the message test.DependencyMessage.
 * Use `create(DependencyMessageSchema)` to create a new message.
 */
export const DependencyMessageSchema: GenMessage<DependencyMessage> = /*@__PURE__*/
  messageDesc(file_test_schemaregistry_serde_dep, 0);

