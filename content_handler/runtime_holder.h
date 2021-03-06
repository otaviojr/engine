// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_
#define FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_

#include <mx/channel.h>

#include <unordered_set>

#include "application/lib/app/application_context.h"
#include "application/services/application_environment.fidl.h"
#include "application/services/service_provider.fidl.h"
#include "apps/mozart/lib/flutter/sdk_ext/src/natives.h"
#include "apps/mozart/services/input/input_connection.fidl.h"
#include "apps/mozart/services/input/text_input.fidl.h"
#include "apps/mozart/services/views/view_manager.fidl.h"
#include "flutter/assets/unzipper_provider.h"
#include "flutter/assets/zip_asset_store.h"
#include "flutter/flow/layers/layer_tree.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/runtime/runtime_controller.h"
#include "flutter/runtime/runtime_delegate.h"
#include "lib/fidl/cpp/bindings/binding.h"
#include "lib/ftl/functional/closure.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/weak_ptr.h"

namespace flutter_runner {
class Rasterizer;

class RuntimeHolder : public blink::RuntimeDelegate,
                      public mozart::NativesDelegate,
                      public mozart::ViewListener,
                      public mozart::InputListener,
                      public mozart::InputMethodEditorClient {
 public:
  RuntimeHolder();
  ~RuntimeHolder();

  void Init(std::unique_ptr<app::ApplicationContext> context,
            fidl::InterfaceRequest<app::ServiceProvider> outgoing_services,
            std::vector<char> bundle);
  void CreateView(const std::string& script_uri,
                  fidl::InterfaceRequest<mozart::ViewOwner> view_owner_request,
                  fidl::InterfaceRequest<app::ServiceProvider> services);

  Dart_Port GetUIIsolateMainPort();
  std::string GetUIIsolateName();

 private:
  // |blink::RuntimeDelegate| implementation:
  void ScheduleFrame() override;
  void Render(std::unique_ptr<flow::LayerTree> layer_tree) override;
  void UpdateSemantics(std::vector<blink::SemanticsNode> update) override;
  void HandlePlatformMessage(
      ftl::RefPtr<blink::PlatformMessage> message) override;
  void DidCreateMainIsolate(Dart_Isolate isolate) override;

  // |mozart::NativesDelegate| implementation:
  mozart::View* GetMozartView() override;

  // |mozart::InputListener| implementation:
  void OnEvent(mozart::InputEventPtr event,
               const OnEventCallback& callback) override;

  // |mozart::ViewListener| implementation:
  void OnInvalidation(mozart::ViewInvalidationPtr invalidation,
                      const OnInvalidationCallback& callback) override;

  // |mozart::InputMethodEditorClient| implementation:
  void DidUpdateState(mozart::TextInputStatePtr state,
                      mozart::InputEventPtr event) override;
  void OnAction(mozart::InputMethodAction action) override;

  ftl::WeakPtr<RuntimeHolder> GetWeakPtr();

  void InitRootBundle(std::vector<char> bundle);
  blink::UnzipperProvider GetUnzipperProviderForRootBundle();
  bool HandleAssetPlatformMessage(blink::PlatformMessage* message);
  bool HandleTextInputPlatformMessage(blink::PlatformMessage* message);

  void InitFidlInternal();
  void InitMozartInternal();

  void BeginFrame();
  void OnFrameComplete();
  void Invalidate();

  std::unique_ptr<app::ApplicationContext> context_;
  fidl::InterfaceRequest<app::ServiceProvider> outgoing_services_;

  std::vector<char> root_bundle_data_;
  ftl::RefPtr<blink::ZipAssetStore> asset_store_;
  void* dylib_handle_ = nullptr;

  std::unique_ptr<Rasterizer> rasterizer_;
  std::unique_ptr<blink::RuntimeController> runtime_;
  blink::ViewportMetrics viewport_metrics_;

  mozart::ViewManagerPtr view_manager_;
  fidl::Binding<mozart::ViewListener> view_listener_binding_;
  fidl::Binding<mozart::InputListener> input_listener_binding_;
  mozart::InputConnectionPtr input_connection_;
  mozart::ViewPtr view_;
  mozart::ViewPropertiesPtr view_properties_;
  uint32_t scene_version_ = mozart::kSceneVersionNone;

  std::unordered_set<int> down_pointers_;

  bool pending_invalidation_ = false;
  OnInvalidationCallback deferred_invalidation_callback_;
  bool is_ready_to_draw_ = false;
  int outstanding_requests_ = 0;

  mozart::InputMethodEditorPtr input_method_editor_;
  fidl::Binding<mozart::InputMethodEditorClient> text_input_binding_;
  int current_text_input_client_ = 0;

  ftl::WeakPtrFactory<RuntimeHolder> weak_factory_;

  FTL_DISALLOW_COPY_AND_ASSIGN(RuntimeHolder);
};

}  // namespace flutter_runner

#endif  // FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_
