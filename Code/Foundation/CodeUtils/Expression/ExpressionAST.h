#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Memory/LinearAllocator.h>

class nsDGMLGraph;

class NS_FOUNDATION_DLL nsExpressionAST
{
public:
  struct NodeType
  {
    using StorageType = nsUInt8;

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
      SmoothStep,
      SmootherStep,
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
    using StorageType = nsUInt8;

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

    static nsVariantType::Enum GetVariantType(Enum dataType);

    static Enum FromStreamType(nsProcessingStream::DataType dataType);

    NS_ALWAYS_INLINE static nsExpression::RegisterType::Enum GetRegisterType(Enum dataType)
    {
      return static_cast<nsExpression::RegisterType::Enum>(dataType >> 2);
    }

    NS_ALWAYS_INLINE static Enum FromRegisterType(nsExpression::RegisterType::Enum registerType, nsUInt32 uiElementCount = 1)
    {
      return static_cast<nsExpressionAST::DataType::Enum>((registerType << 2) + uiElementCount - 1);
    }

    NS_ALWAYS_INLINE static nsUInt32 GetElementCount(Enum dataType) { return (dataType & 0x3) + 1; }

    static const char* GetName(Enum dataType);
  };

  struct VectorComponent
  {
    using StorageType = nsUInt8;

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

    static Enum FromChar(nsUInt32 uiChar);
  };

  struct Node
  {
    nsEnum<NodeType> m_Type;
    nsEnum<DataType> m_ReturnType;
    nsUInt8 m_uiOverloadIndex = 0xFF;
    nsUInt8 m_uiNumInputElements = 0;

    nsUInt32 m_uiHash = 0;
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
    nsVariant m_Value;
  };

  struct Swizzle : public Node
  {
    nsEnum<VectorComponent> m_Components[4];
    nsUInt32 m_NumComponents = 0;
    Node* m_pExpression = nullptr;
  };

  struct Input : public Node
  {
    nsExpression::StreamDesc m_Desc;
  };

  struct Output : public Node
  {
    nsExpression::StreamDesc m_Desc;
    Node* m_pExpression = nullptr;
  };

  struct FunctionCall : public Node
  {
    nsSmallArray<const nsExpression::FunctionDesc*, 1> m_Descs;
    nsSmallArray<Node*, 8> m_Arguments;
  };

  struct ConstructorCall : public Node
  {
    nsSmallArray<Node*, 4> m_Arguments;
  };

public:
  nsExpressionAST();
  ~nsExpressionAST();

  UnaryOperator* CreateUnaryOperator(NodeType::Enum type, Node* pOperand, DataType::Enum returnType = DataType::Unknown);
  BinaryOperator* CreateBinaryOperator(NodeType::Enum type, Node* pLeftOperand, Node* pRightOperand);
  TernaryOperator* CreateTernaryOperator(NodeType::Enum type, Node* pFirstOperand, Node* pSecondOperand, Node* pThirdOperand);
  Constant* CreateConstant(const nsVariant& value, DataType::Enum dataType = DataType::Float);
  Swizzle* CreateSwizzle(nsStringView sSwizzle, Node* pExpression);
  Swizzle* CreateSwizzle(nsEnum<VectorComponent> component, Node* pExpression);
  Swizzle* CreateSwizzle(nsArrayPtr<nsEnum<VectorComponent>> swizzle, Node* pExpression);
  Input* CreateInput(const nsExpression::StreamDesc& desc);
  Output* CreateOutput(const nsExpression::StreamDesc& desc, Node* pExpression);
  FunctionCall* CreateFunctionCall(const nsExpression::FunctionDesc& desc, nsArrayPtr<Node*> arguments);
  FunctionCall* CreateFunctionCall(nsArrayPtr<const nsExpression::FunctionDesc> descs, nsArrayPtr<Node*> arguments);
  ConstructorCall* CreateConstructorCall(DataType::Enum dataType, nsArrayPtr<Node*> arguments);
  ConstructorCall* CreateConstructorCall(Node* pOldValue, Node* pNewValue, nsStringView sPartialAssignmentMask);

  static nsArrayPtr<Node*> GetChildren(Node* pNode);
  static nsArrayPtr<const Node*> GetChildren(const Node* pNode);

  void PrintGraph(nsDGMLGraph& inout_graph) const;

  nsSmallArray<Input*, 8> m_InputNodes;
  nsSmallArray<Output*, 8> m_OutputNodes;

  // Transforms
  Node* TypeDeductionAndConversion(Node* pNode);
  Node* ReplaceVectorInstructions(Node* pNode);
  Node* ScalarizeVectorInstructions(Node* pNode);
  Node* ReplaceUnsupportedInstructions(Node* pNode);
  Node* FoldConstants(Node* pNode);
  Node* CommonSubexpressionElimination(Node* pNode);
  Node* Validate(Node* pNode);

  nsResult ScalarizeInputs();
  nsResult ScalarizeOutputs();

private:
  void ResolveOverloads(Node* pNode);

  static DataType::Enum GetExpectedChildDataType(const Node* pNode, nsUInt32 uiChildIndex);

  static void UpdateHash(Node* pNode);
  static bool IsEqual(const Node* pNodeA, const Node* pNodeB);

  nsLinearAllocator<> m_Allocator;

  nsSet<nsExpression::FunctionDesc> m_FunctionDescs;

  nsHashTable<nsUInt32, nsSmallArray<Node*, 1>> m_NodeDeduplicationTable;
};
