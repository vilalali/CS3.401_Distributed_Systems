# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: knn.proto
# Protobuf Python Version: 5.27.2
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import runtime_version as _runtime_version
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
_runtime_version.ValidateProtobufRuntimeVersion(
    _runtime_version.Domain.PUBLIC,
    5,
    27,
    2,
    '',
    'knn.proto'
)
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\tknn.proto\x12\x03knn\"+\n\nKNNRequest\x12\x12\n\ndata_point\x18\x01 \x03(\x02\x12\t\n\x01k\x18\x02 \x01(\x05\"8\n\x0bKNNResponse\x12)\n\x11nearest_neighbors\x18\x01 \x03(\x0b\x32\x0e.knn.DataPoint\" \n\tDataPoint\x12\x13\n\x0b\x63oordinates\x18\x01 \x03(\x02\x32>\n\nKNNService\x12\x30\n\x0b\x46indNearest\x12\x0f.knn.KNNRequest\x1a\x10.knn.KNNResponseb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'knn_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_KNNREQUEST']._serialized_start=18
  _globals['_KNNREQUEST']._serialized_end=61
  _globals['_KNNRESPONSE']._serialized_start=63
  _globals['_KNNRESPONSE']._serialized_end=119
  _globals['_DATAPOINT']._serialized_start=121
  _globals['_DATAPOINT']._serialized_end=153
  _globals['_KNNSERVICE']._serialized_start=155
  _globals['_KNNSERVICE']._serialized_end=217
# @@protoc_insertion_point(module_scope)
