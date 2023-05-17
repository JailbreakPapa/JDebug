#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Memory/StackAllocator.h>

class wdDGMLGraph;

class WD_FOUNDATION_DLL wdExpressionAST
{
public:
  struct NodeType
  {
    using StorageType = wdUInt8;

    enum Enum
    {
      Invalid,
      Default = Invalid,

      // Unary
      FirstUnary,
      Negate,
      Absolute,
      Saturate,
      Sqrt,
      Exp,
      Ln,
      Log2,
      Log10,
      Pow2,
      Sin,
      Cos,
      Tan,
      ASin,
      ACos,
      ATan,
      RadToDeg,
      DegToRad,
      Round,
      Floor,
      Ceil,
      Trunc,
      Frac,
      Length,
      Normalize,
      BitwiseNot,
      LogicalNot,
      All,
      Any,
      TypeConversion,
      LastUnary,

      // Binary
      FirstBinary,
      Add,
      Subtract,
      Multiply,
      Divide,
      Modulo,
      Log,
      Pow,
      Min,
      Max,
      Dot,
      Cross,
      Reflect,
      BitshiftLeft,
      BitshiftRight,
      BitwiseAnd,
      BitwiseXor,
      BitwiseOr,
      Equal,
      NotEqual,
      Less,
      LessEqual,
      Greater,
      GreaterEqual,
      LogicalAnd,
      LogicalOr,
      LastBinary,

      // Ternary
      FirstTernary,
      Clamp,
      Select,
      Lerp,
      LastTernary,

      Constant,
      Swizzle,
      Input,
      Output,

      FunctionCall,
      ConstructorCall,

      Count
    };

    static bool IsUnary(Enum nodeType);
    static bool IsBinary(Enum nodeType);
    static bool IsTernary(Enum nodeType);
    static bool IsConstant(Enum nodeType);
    static bool IsSwizzle(Enum nodeType);
    static bool IsInput(Enum nodeType);
    static bool IsOutput(Enum nodeType);
    static bool IsFunctionCall(Enum nodeType);
    static bool IsConstructorCall(Enum nodeType);

    static bool IsCommutative(Enum nodeType);
    static bool AlwaysReturnsSingleElement(Enum nodeType);

    static const char* GetName(Enum nodeType);
  };

  struct DataType
  {
    using StorageType = wdUInt8;

    enum Enum
    {
      Unknown,
      Unknown2,
      Unknown3,
      Unknown4,

      Bool,
      Bool2,
      Bool3,
      Bool4,

      Int,
      Int2,
      Int3,
      Int4,

      Float,
      Float2,
      Float3,
      Float4,

      Count,

      Default = Unknown,
    };

    static wdVariantType::Enum GetVariantType(Enum dataType);

    static Enum FromStreamType(wdProcessingStream::DataType dataType);

    WD_ALWAYS_INLINE static wdExpression::RegisterType::Enum GetRegisterType(Enum dataType)
    {
      return static_cast<wdExpression::RegisterType::Enum>(dataType >> 2);
    }

    WD_ALWAYS_INLINE static Enum FromRegisterType(wdExpression::RegisterType::Enum registerType, wdUInt32 uiElementCount = 1)
    {
      return static_cast<wdExpressionAST::DataType::Enum>((registerType << 2) + uiElementCount - 1);
    }

    WD_ALWAYS_INLINE static wdUInt32 GetElementCount(Enum dataType) { return (dataType & 0x3) + 1; }

    static const char* GetName(Enum dataType);
  };

  struct VectorComponent
  {
    using StorageType = wdUInt8;

    enum Enum
    {
      X,
      Y,
      Z,
      W,

      R = X,
      G = Y,
      B = Z,
      A = W,

      Count,

      Default = X
    };

    static const char* GetName(Enum vectorComponent);

    static Enum FromChar(wdUInt32 uiChar);
  };

  struct Node
  {
    wdEnum<NodeType> m_Type;
    wdEnum<DataType> m_ReturnType;
    wdUInt8 m_uiOverloadIndex = 0xFF;
    wdUInt8 m_uiNumInputElements = 0;

    wdUInt32 m_uiHash = 0;
  };

  struct UnaryOperator : public Node
  {
    Node* m_pOperand = nullptr;
  };

  struct BinaryOperator : public Node
  {
    Node* m_pLeftOperand = nullptr;
    Node* m_pRightOperand = nullptr;
  };

  struct TernaryOperator : public Node
  {
    Node* m_pFirstOperand = nullptr;
    Node* m_pSecondOperand = nullptr;
    Node* m_pThirdOperand = nullptr;
  };

  struct Constant : public Node
  {
    wdVariant m_Value;
  };

  struct Swizzle : public Node
  {
    wdEnum<VectorComponent> m_Components[4];
    wdUInt32 m_NumComponents = 0;
    Node* m_pExpression = nullptr;
  };

  struct Input : public Node
  {
    wdExpression::StreamDesc m_Desc;
  };

  struct Output : public Node
  {
    wdExpression::StreamDesc m_Desc;
    Node* m_pExpression = nullptr;
  };

  struct FunctionCall : public Node
  {
    wdSmallArray<const wdExpression::FunctionDesc*, 1> m_Descs;
    wdSmallArray<Node*, 8> m_Arguments;
  };

  struct ConstructorCall : public Node
  {
    wdSmallArray<Node*, 4> m_Arguments;
  };

public:
  wdExpressionAST();
  ~wdExpressionAST();

  UnaryOperator* CreateUnaryOperator(NodeType::Enum type, Node* pOperand, DataType::Enum returnType = DataType::Unknown);
  BinaryOperator* CreateBinaryOperator(NodeType::Enum type, Node* pLeftOperand, Node* pRightOperand);
  TernaryOperator* CreateTernaryOperator(NodeType::Enum type, Node* pFirstOperand, Node* pSecondOperand, Node* pThirdOperand);
  Constant* CreateConstant(const wdVariant& value, DataType::Enum dataType = DataType::Float);
  Swizzle* CreateSwizzle(wdStringView sSwizzle, Node* pExpression);
  Swizzle* CreateSwizzle(wdEnum<VectorComponent> component, Node* pExpression);
  Swizzle* CreateSwizzle(wdArrayPtr<wdEnum<VectorComponent>> swizzle, Node* pExpression);
  Input* CreateInput(const wdExpression::StreamDesc& desc);
  Output* CreateOutput(const wdExpression::StreamDesc& desc, Node* pExpression);
  FunctionCall* CreateFunctionCall(const wdExpression::FunctionDesc& desc, wdArrayPtr<Node*> arguments);
  FunctionCall* CreateFunctionCall(wdArrayPtr<const wdExpression::FunctionDesc> descs, wdArrayPtr<Node*> arguments);
  ConstructorCall* CreateConstructorCall(DataType::Enum dataType, wdArrayPtr<Node*> arguments);
  ConstructorCall* CreateConstructorCall(Node* pOldValue, Node* pNewValue, wdStringView sPartialAssignmentMask);

  static wdArrayPtr<Node*> GetChildren(Node* pNode);
  static wdArrayPtr<const Node*> GetChildren(const Node* pNode);

  void PrintGraph(wdDGMLGraph& inout_graph) const;

  wdHybridArray<Output*, 8> m_OutputNodes;

  // Transforms
  Node* TypeDeductionAndConversion(Node* pNode);
  Node* ReplaceVectorInstructions(Node* pNode);
  Node* ScalarizeVectorInstructions(Node* pNode);
  Node* ReplaceUnsupportedInstructions(Node* pNode);
  Node* FoldConstants(Node* pNode);
  Node* CommonSubexpressionElimination(Node* pNode);
  Node* Validate(Node* pNode);

  wdResult ScalarizeOutputs();

private:
  void ResolveOverloads(Node* pNode);

  static DataType::Enum GetExpectedChildDataType(const Node* pNode, wdUInt32 uiChildIndex);

  static void UpdateHash(Node* pNode);
  static bool IsEqual(const Node* pNodeA, const Node* pNodeB);

  wdStackAllocator<> m_Allocator;

  wdSet<wdExpression::FunctionDesc> m_FunctionDescs;

  wdHashTable<wdUInt32, wdSmallArray<Node*, 1>> m_NodeDeduplicationTable;
};
