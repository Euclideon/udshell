#pragma once
#if !defined(_EP_IRESOURCE_HPP)
#define _EP_IRESOURCE_HPP

namespace ep {

class IResource
{
public:
  virtual ComponentRef GetMetadata() const = 0; // TODO Change ComponentRef to Metadata once Metadata made public
  virtual ComponentRef GetDataSource() const = 0; // TODO Change ComponentRef to DataSourceRef once DataSource made public
};

}

#endif // _EP_RESOURCE_H
