// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';
import 'dart:convert';

import 'package:mojo/bindings.dart' as bindings;
import 'package:mojo/core.dart' as core;
import 'package:_mojo_for_test_only/expect.dart';
import 'package:_mojo_for_test_only/mojo/test/rect.mojom.dart' as rect;
import 'package:_mojo_for_test_only/mojo/test/serialization_test_structs.mojom.dart'
    as serialization;
import 'package:_mojo_for_test_only/mojo/test/test_structs.mojom.dart'
    as structs;
import 'package:_mojo_for_test_only/mojo/test/test_unions.mojom.dart'
    as unions;
import 'package:_mojo_for_test_only/regression_tests/regression_tests.mojom.dart'
    as regression;
import 'package:_mojo_for_test_only/sample/sample_interfaces.mojom.dart'
    as sample;

class ProviderImpl implements sample.Provider {
  sample.ProviderStub _stub;

  ProviderImpl(core.MojoMessagePipeEndpoint endpoint) {
    _stub = new sample.ProviderStub.fromEndpoint(endpoint, this);
  }

  echoString(String a, Function responseFactory) =>
      new Future.value(responseFactory(a));

  echoStrings(String a, String b, Function responseFactory) =>
      new Future.value(responseFactory(a, b));

  echoMessagePipeHanlde(core.MojoHandle a, Function responseFactory) =>
      new Future.value(responseFactory(a));

  echoEnum(sample.Enum a, Function responseFactory) =>
      new Future.value(responseFactory(a));
}

void providerIsolate(core.MojoMessagePipeEndpoint endpoint) {
  new ProviderImpl(endpoint);
}

Future<bool> testCallResponse() {
  var pipe = new core.MojoMessagePipe();
  var client = new sample.ProviderProxy.fromEndpoint(pipe.endpoints[0]);
  var c = new Completer();
  Isolate.spawn(providerIsolate, pipe.endpoints[1]).then((_) {
    client.ptr.echoString("hello!").then((echoStringResponse) {
      Expect.equals("hello!", echoStringResponse.a);
    }).then((_) {
      client.ptr.echoStrings("hello", "mojo!").then((echoStringsResponse) {
        Expect.equals("hello", echoStringsResponse.a);
        Expect.equals("mojo!", echoStringsResponse.b);
        client.close().then((_) {
          c.complete(true);
        });
      });
    });
  });
  return c.future;
}

Future testAwaitCallResponse() async {
  var pipe = new core.MojoMessagePipe();
  var client = new sample.ProviderProxy.fromEndpoint(pipe.endpoints[0]);
  var isolate = await Isolate.spawn(providerIsolate, pipe.endpoints[1]);

  var echoStringResponse = await client.ptr.echoString("hello!");
  Expect.equals("hello!", echoStringResponse.a);

  var echoStringsResponse = await client.ptr.echoStrings("hello", "mojo!");
  Expect.equals("hello", echoStringsResponse.a);
  Expect.equals("mojo!", echoStringsResponse.b);

  await client.close();
}

bindings.ServiceMessage messageOfStruct(bindings.Struct s) =>
    s.serializeWithHeader(new bindings.MessageHeader(0));

testSerializeNamedRegion() {
  var r = new rect.Rect()
    ..x = 1
    ..y = 2
    ..width = 3
    ..height = 4;
  var namedRegion = new structs.NamedRegion()
    ..name = "name"
    ..rects = [r];
  var message = messageOfStruct(namedRegion);
  var namedRegion2 = structs.NamedRegion.deserialize(message.payload);
  Expect.equals(namedRegion.name, namedRegion2.name);
}

testSerializeArrayValueTypes() {
  var arrayValues = new structs.ArrayValueTypes()
    ..f0 = [0, 1, -1, 0x7f, -0x10]
    ..f1 = [0, 1, -1, 0x7fff, -0x1000]
    ..f2 = [0, 1, -1, 0x7fffffff, -0x10000000]
    ..f3 = [0, 1, -1, 0x7fffffffffffffff, -0x1000000000000000]
    ..f4 = [0.0, 1.0, -1.0, 4.0e9, -4.0e9]
    ..f5 = [0.0, 1.0, -1.0, 4.0e9, -4.0e9];
  var message = messageOfStruct(arrayValues);
  var arrayValues2 = structs.ArrayValueTypes.deserialize(message.payload);
  Expect.listEquals(arrayValues.f0, arrayValues2.f0);
  Expect.listEquals(arrayValues.f1, arrayValues2.f1);
  Expect.listEquals(arrayValues.f2, arrayValues2.f2);
  Expect.listEquals(arrayValues.f3, arrayValues2.f3);
  Expect.listEquals(arrayValues.f4, arrayValues2.f4);
  Expect.listEquals(arrayValues.f5, arrayValues2.f5);
}

testSerializeToJSON() {
  var r = new rect.Rect()
    ..x = 1
    ..y = 2
    ..width = 3
    ..height = 4;

  var encodedRect = JSON.encode(r);
  var goldenEncoding = "{\"x\":1,\"y\":2,\"width\":3,\"height\":4}";
  Expect.equals(goldenEncoding, encodedRect);
}

testSerializeHandleToJSON() {
  var s = new serialization.Struct2();

  Expect.throws(
      () => JSON.encode(s), (e) => e.cause is bindings.MojoCodecError);
}

testSerializeKeywordStruct() {
  var keywordStruct = new structs.DartKeywordStruct()
      ..await_ = structs.DartKeywordStructKeywords.await_
      ..is_ = structs.DartKeywordStructKeywords.is_
      ..rethrow_ = structs.DartKeywordStructKeywords.rethrow_;
  var message = messageOfStruct(keywordStruct);
  var decodedStruct = structs.DartKeywordStruct.deserialize(message.payload);
  Expect.equals(keywordStruct.await_, decodedStruct.await_);
  Expect.equals(keywordStruct.is_, decodedStruct.is_);
  Expect.equals(keywordStruct.rethrow_, decodedStruct.rethrow_);
}

testSerializeStructs() {
  testSerializeNamedRegion();
  testSerializeArrayValueTypes();
  testSerializeToJSON();
  testSerializeHandleToJSON();
  testSerializeKeywordStruct();
}

testSerializePodUnions() {
  var s = new unions.WrapperStruct()..podUnion = new unions.PodUnion();
  s.podUnion.fUint32 = 32;

  Expect.equals(unions.PodUnionTag.fUint32, s.podUnion.tag);
  Expect.equals(32, s.podUnion.fUint32);

  var message = messageOfStruct(s);
  var s2 = unions.WrapperStruct.deserialize(message.payload);

  Expect.equals(s.podUnion.fUint32, s2.podUnion.fUint32);
}

testSerializeStructInUnion() {
  var s = new unions.WrapperStruct()..objectUnion = new unions.ObjectUnion();
  s.objectUnion.fDummy = new unions.DummyStruct()..fInt8 = 8;

  var message = messageOfStruct(s);
  var s2 = unions.WrapperStruct.deserialize(message.payload);

  Expect.equals(s.objectUnion.fDummy.fInt8, s2.objectUnion.fDummy.fInt8);
}

testSerializeArrayInUnion() {
  var s = new unions.WrapperStruct()..objectUnion = new unions.ObjectUnion();
  s.objectUnion.fArrayInt8 = [1, 2, 3];

  var message = messageOfStruct(s);
  var s2 = unions.WrapperStruct.deserialize(message.payload);

  Expect.listEquals(s.objectUnion.fArrayInt8, s2.objectUnion.fArrayInt8);
}

testSerializeMapInUnion() {
  var s = new unions.WrapperStruct()..objectUnion = new unions.ObjectUnion();
  s.objectUnion.fMapInt8 = {"one": 1, "two": 2,};

  var message = messageOfStruct(s);
  var s2 = unions.WrapperStruct.deserialize(message.payload);

  Expect.equals(1, s.objectUnion.fMapInt8["one"]);
  Expect.equals(2, s.objectUnion.fMapInt8["two"]);
}

testSerializeUnionInArray() {
  var s = new unions.SmallStruct()
    ..podUnionArray = [
      new unions.PodUnion()..fUint16 = 16,
      new unions.PodUnion()..fUint32 = 32,
    ];

  var message = messageOfStruct(s);

  var s2 = unions.SmallStruct.deserialize(message.payload);

  Expect.equals(16, s2.podUnionArray[0].fUint16);
  Expect.equals(32, s2.podUnionArray[1].fUint32);
}

testSerializeUnionInMap() {
  var s = new unions.SmallStruct()
    ..podUnionMap = {
      'one': new unions.PodUnion()..fUint16 = 16,
      'two': new unions.PodUnion()..fUint32 = 32,
    };

  var message = messageOfStruct(s);

  var s2 = unions.SmallStruct.deserialize(message.payload);

  Expect.equals(16, s2.podUnionMap['one'].fUint16);
  Expect.equals(32, s2.podUnionMap['two'].fUint32);
}

testSerializeUnionInUnion() {
  var s = new unions.WrapperStruct()..objectUnion = new unions.ObjectUnion();
  s.objectUnion.fPodUnion = new unions.PodUnion()..fUint32 = 32;

  var message = messageOfStruct(s);
  var s2 = unions.WrapperStruct.deserialize(message.payload);

  Expect.equals(32, s2.objectUnion.fPodUnion.fUint32);
}

testUnionsToString() {
  var podUnion = new unions.PodUnion();
  podUnion.fUint32 = 32;
  Expect.equals("PodUnion(fUint32: 32)", podUnion.toString());
}

testUnions() {
  testSerializePodUnions();
  testSerializeStructInUnion();
  testSerializeArrayInUnion();
  testSerializeMapInUnion();
  testSerializeUnionInArray();
  testSerializeUnionInMap();
  testSerializeUnionInUnion();
  testUnionsToString();
}

class CheckEnumCapsImpl implements regression.CheckEnumCaps {
  regression.CheckEnumCapsStub _stub;

  CheckEnumCapsImpl(core.MojoMessagePipeEndpoint endpoint) {
    _stub = new regression.CheckEnumCapsStub.fromEndpoint(endpoint, this);
  }

  setEnumWithInternalAllCaps(regression.EnumWithInternalAllCaps e) {}
}

checkEnumCapsIsolate(core.MojoMessagePipeEndpoint endpoint) {
  new CheckEnumCapsImpl(endpoint);
}

testCheckEnumCapsImpl() {
  var pipe = new core.MojoMessagePipe();
  var client =
      new regression.CheckEnumCapsProxy.fromEndpoint(pipe.endpoints[0]);
  var c = new Completer();
  Isolate.spawn(checkEnumCapsIsolate, pipe.endpoints[1]).then((_) {
    client.ptr.setEnumWithInternalAllCaps(
        regression.EnumWithInternalAllCaps.standard);
    client.close().then((_) {
      c.complete(null);
    });
  });
  return c.future;
}

testSerializeEnum() {
  var constants = new structs.ScopedConstants();
  constants.f4 = structs.ScopedConstantsEType.e0;
  var message = messageOfStruct(constants);
  var constants2 = structs.ScopedConstants.deserialize(message.payload);
  Expect.equals(constants.f4, constants2.f4);
}

testEnums() async {
  testSerializeEnum();
  await testCheckEnumCapsImpl();
}

void closingProviderIsolate(core.MojoMessagePipeEndpoint endpoint) {
  var provider = new ProviderImpl(endpoint);
  provider._stub.close();
}

Future<bool> runOnClosedTest() {
  var testCompleter = new Completer();
  var pipe = new core.MojoMessagePipe();
  var proxy = new sample.ProviderProxy.fromEndpoint(pipe.endpoints[0]);
  proxy.impl.onError = (_) => testCompleter.complete(true);
  Isolate.spawn(closingProviderIsolate, pipe.endpoints[1]);
  return testCompleter.future.then((b) {
    Expect.isTrue(b);
  });
}

class Regression551Impl implements regression.Regression551 {
  regression.Regression551Stub _stub;

  Regression551Impl(core.MojoMessagePipeEndpoint endpoint) {
    _stub = new regression.Regression551Stub.fromEndpoint(endpoint, this);
  }

  dynamic get(List<String> keyPrefixes, Function responseFactory) =>
    responseFactory(0);
}

void regression551Isolate(core.MojoMessagePipeEndpoint endpoint) {
  new Regression551Impl(endpoint);
}

Future<bool> testRegression551() {
  var pipe = new core.MojoMessagePipe();
  var client = new regression.Regression551Proxy.fromEndpoint(pipe.endpoints[0]);
  var c = new Completer();
  Isolate.spawn(regression551Isolate, pipe.endpoints[1]).then((_) {
    client.ptr.get(["hello!"]).then((response) {
      Expect.equals(0, response.result);
      client.close().then((_) {
        c.complete(true);
      });
    });
  });
  return c.future;
}

class ServiceNameImpl implements regression.ServiceName {
  regression.ServiceNameStub _stub;

  ServiceNameImpl(core.MojoMessagePipeEndpoint endpoint) {
    _stub = new regression.ServiceNameStub.fromEndpoint(endpoint, this);
  }

  dynamic serviceName_(Function responseFactory) =>
      responseFactory(ServiceName.serviceName);
}

void serviceNameIsolate(core.MojoMessagePipeEndpoint endpoint) {
  new ServiceNameImpl(endpoint);
}

Future<bool> testServiceName() {
  var pipe = new core.MojoMessagePipe();
  var client = new regression.ServiceNameProxy.fromEndpoint(pipe.endpoints[0]);
  var c = new Completer();
  Isolate.spawn(serviceNameIsolate, pipe.endpoints[1]).then((_) {
    client.ptr.serviceName_().then((response) {
      Expect.equals(ServiceName.serviceName, response.serviceName_);
      client.close().then((_) {
        c.complete(true);
      });
    });
  });
  return c.future;
}


testCamelCase() {
  var e = CamelCaseTestEnum.boolThing;
  e = CamelCaseTestEnum.doubleThing;
  e = CamelCaseTestEnum.floatThing;
  e = CamelCaseTestEnum.int8Thing;
  e = CamelCaseTestEnum.int16Thing;
  e = CamelCaseTestEnum.int32Th1Ng;
  e = CamelCaseTestEnum.int64Th1ng;
  e = CamelCaseTestEnum.uint8TH1ng;
  e = CamelCaseTestEnum.uint16tH1Ng;
  e = CamelCaseTestEnum.uint32Th1ng;
  e = CamelCaseTestEnum.uint64Th1Ng;
}


main() async {
  testSerializeStructs();
  testUnions();
  await testEnums();
  await testCallResponse();
  await testAwaitCallResponse();
  await runOnClosedTest();
  await testRegression551();
  await testServiceName();
  testCamelCase();
}
