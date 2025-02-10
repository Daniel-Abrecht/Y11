#!/usr/bin/env python3

import re
import os

messages = dict()

class Message:
  def __init__(self, name, alignment):
    self.name = name
    self.alignment = alignment
    self.fields = []
    messages['y11_msg_'+name+'_t'] = self
  def append_field(self, field):
    self.fields.append(field)
  def get_size(self):
    return sum(x.get_size() for x in self.fields)
  def __str__(self):
    return self.name + ''.join(['\n  '+str(f) for f in self.fields])

class Field:
  def __init__(self, content):
    result = re.match(r'([^\0]*) ([_a-zA-Z][_a-zA-Z0-9]*) *(\[ *([0-9]+) *\])? *$', content, re.MULTILINE).groups()
    self.type = re.sub('[ \n\t\v]+', ' ', result[0])
    self.name = result[1]
    self.is_array = result[3] is not None
    self.count = int(result[3]) if result[3] is not None else 1
  def get_size(self):
    return self.get_item_size() * self.count
  def get_alignment(self):
    if self.type in messages:
      return messages.get(self.type).alignment
    return self.get_item_size()
  def get_item_size(self):
    match self.type:
      case 'char' | 'signed char' | 'unsigned char' | 'int8_t' | 'uint8_t':
        return 1
      case 'int16_t' | 'uint16_t':
        return 2
      case 'int32_t' | 'uint32_t':
        return 4
      case 'int64_t' | 'uint64_t':
        return 8
      case _:
        return messages.get(self.type).get_size()
  def __str__(self):
    return f'{self.count, self.name, self.type}'

def load(path):
  with open(path,'r') as content:
    content = content.read()
  content = re.sub(r'//.*|/\*([^*]/|[^/])*\*/','',content);
  regex = re.compile(r"^Y11_MESSAGE(_COMPONENT)?\(([^,]*), *([^,]*), *\(([^\0]*?)^\)\)", re.MULTILINE)
  for _, name, alignment, content in regex.findall(content):
    message = Message(name.strip(), int(alignment.strip()))
    for x in content.split(';'):
      x = x.strip()
      if x:
        message.append_field(Field(x))
    messages['y11_msg_'+message.name+'_t'] = message

def validate():
  errors = []
  for message in messages.values():
    off = 0
    message_alignment = message.alignment
    for field in message.fields:
      field_alignment = field.get_alignment()
      if off % field_alignment:
        errors.append(f"Field {field.name} of message y11_msg_{message.name}_t is not aligned properly (offset {off})")
      if message_alignment % field_alignment:
        errors.append(f"Type of field {field.name} of message y11_msg_{message.name}_t needs a larger alignment then specified for the message ({message_alignment} mod {field_alignment} != 0)")
      off += field.get_size()
    if off % message_alignment:
      errors.append(f"Message y11_msg_{message.name}_t lacks {message_alignment - (off % message_alignment)} bytes of padding");
  if errors:
    raise Exception('\n * '.join(["Validation failed:",*errors]))

def gen_assertions():
  for message in messages.values():
    off = 0
    for field in message.fields:
      yield f'static_assert(offsetof(y11_msg_{message.name}_t,{field.name}) == {off}, "Field of packed struct has unexpected offset!");\n'
      off += field.get_size()

def gen_swap_functions_c():
  for message in messages.values():
    yield f'extern y11_msg_{message.name}_t* y11_msg_swap_endianess__{message.name}(y11_msg_{message.name}_t*restrict const msg);\n'
  yield '\n'

def gen_swap_functions_h():
  for message in messages.values():
    yield f'inline y11_msg_{message.name}_t* y11_msg_swap_endianess__{message.name}(y11_msg_{message.name}_t*restrict const msg);\n'
  yield '\n'
  for message in messages.values():
    yield f'inline y11_msg_{message.name}_t* y11_msg_swap_endianess__{message.name}(y11_msg_{message.name}_t*restrict const msg){{\n'
    fields = message.fields
    # if fields[0].type == 'y11_msg_header_t':
    #   fields = fields[1:]
    for field in fields:
      loop=''
      index=''
      if field.is_array:
        loop=f'  for(int i=0; i<{field.count}; i++)\n  '
        index = '[i]'
      match field.type:
        case 'char' | 'signed char' | 'unsigned char' | 'int8_t' | 'uint8_t':
          pass
        case 'int16_t' | 'uint16_t':
          yield f'{loop}  msg->{field.name}{index} = bswap_16(msg->{field.name}{index});\n'
        case 'int32_t' | 'uint32_t':
          yield f'{loop}  msg->{field.name}{index} = bswap_32(msg->{field.name}{index});\n'
        case 'int64_t' | 'uint64_t':
          yield f'{loop}  msg->{field.name}{index} = bswap_64(msg->{field.name}{index});\n'
        case _:
          msg = messages.get(field.type)
          yield f'{loop}  y11_msg_swap_endianess__{msg.name}(&msg->{field.name}{index});\n'
    yield '  return msg;\n'
    yield '}\n\n'

def gen_code_headers():
  yield """\
#ifndef Y11_PROTOCOL_H
#define Y11_PROTOCOL_H

#include <Y11/protocol.common.h>
#include <assert.h>
#include <stddef.h>

"""
  yield from gen_assertions()
  max_size = sum(message.get_size() for message in messages.values())
  yield f"\n#define Y11_MESSAGE_HEADER_SIZE_MAX {max_size}\n"
  yield "\n#endif\n"

def gen_code_headers_private():
  yield """\
#ifndef Y11__PROTOCOL_H
#define Y11__PROTOCOL_H

#include <Y11/protocol.h>
#include <byteswap.h>

"""
  yield from gen_swap_functions_h()
  yield "\n#endif\n"

def gen_code_sources():
  yield """\
#include <-Y11/protocol.h>

"""
  yield from gen_swap_functions_c()

load("include/Y11/protocol.msg.h")
validate()

def gen_code(f, gen):
  os.makedirs(os.path.dirname(f), 0o777, True)
  code = ''.join(gen())
  with open(f, 'w') as f:
    f.write(code)

gen_code('bin/include/Y11/protocol.h', gen_code_headers)
gen_code('build/include/-Y11/protocol.h', gen_code_headers_private)
gen_code('build/src/protocol.c', gen_code_sources)
