<!--
    Teich17.xslt

    translator of CAL to SysteMoC2.0

    author: Juergen Teich
-->

<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:calext="caltrop.dom.xalan.Extension"
  version="1.1">
  <xsl:output method="text"/>


<!--
	Top-Level
-->
<xsl:template match="/">
<xsl:text>
#include &lt;callib.hpp&gt;

</xsl:text>
     <xsl:apply-templates select="//Actor"/>
</xsl:template>


<!--
	Actor generator
-->
<xsl:template match="Actor">
<xsl:text>class m_</xsl:text>
<xsl:value-of select="@name"/>
<xsl:text>: public smoc_actor {
</xsl:text>


<xsl:variable name="port_num" select="count(/Actor/Port)"/>
<xsl:text>// The actor has </xsl:text>
<xsl:value-of select="$port_num"/>
<xsl:text> Ports.
</xsl:text>
<xsl:if test="$port_num > 0">
<xsl:text>public: 
</xsl:text>
<xsl:apply-templates select="/Actor/Port"/>
</xsl:if>


<xsl:variable name="param_num" select="count(/Actor/Decl[@kind='Parameter'])"/>
<xsl:text>
// The actor has </xsl:text>
<xsl:value-of select="$param_num"/>
<xsl:text> Parameters and </xsl:text>
<xsl:variable name="variable_num" select="count(/Actor/Decl[@kind = 'Variable' and (count(./Type[@kind]) = 0)])"/>
<xsl:value-of select="$variable_num"/>
<xsl:text> Variable declarations.
</xsl:text>
<xsl:variable name="vardec_num" select="count(/Actor/Decl[@kind = 'Variable'])"/>
<xsl:if test="$param_num > 0 or $vardec_num > 0">
<xsl:text>private: 
   </xsl:text>
<xsl:apply-templates select="/Actor/Decl"/>
</xsl:if>    



<xsl:variable name="action_num" select="count(/Actor/Action)"/>
<xsl:text>
// The actor has </xsl:text>
<xsl:value-of select="$action_num"/>
<xsl:text> Actions and </xsl:text>
<xsl:variable name="guards_num" select="count(/Actor/Action/Guards)"/>
<xsl:value-of select="$guards_num"/>
<xsl:text> Guards.
</xsl:text>
<xsl:text>private:
</xsl:text>


<xsl:if test="$guards_num > 0">
<xsl:apply-templates select="/Actor/Action/Guards"/>
</xsl:if>
<xsl:if test="$action_num >= 1">
<xsl:apply-templates select="/Actor/Action"/>
</xsl:if>

<xsl:variable name="schedule_fsm" select="count(/Actor/Schedule)"/>
<xsl:choose>
   <xsl:when test="$schedule_fsm > 1">
      <xsl:text>
   ERROR: Actor has more than one schedule_fsm description!
      </xsl:text>
   </xsl:when>
   <xsl:when test="$schedule_fsm = 0">
      <xsl:text>
   smoc_firing_state start;
      </xsl:text>
   </xsl:when>
   <xsl:when test="$schedule_fsm = 1">
      <xsl:text>
   smoc_firing_state </xsl:text>
      <xsl:for-each select="distinct-values(/Actor/Schedule/Transition/@from | /Actor/Schedule/Transition/@to)">
       <xsl:value-of select="."/>
       <xsl:if test="not(position()=last())">
          <xsl:text>, </xsl:text>
       </xsl:if>
       <xsl:if test="(position()=last())">
          <xsl:text>; 
          </xsl:text>
       </xsl:if>
      </xsl:for-each>
   </xsl:when>
</xsl:choose>
<xsl:text>
public:
</xsl:text>
<xsl:text> m_</xsl:text>
<xsl:value-of select="@name"/>
<xsl:text>(sc_module_name name</xsl:text>

<xsl:if test="$param_num > 0">
<xsl:text>, int </xsl:text>
<xsl:for-each select="/Actor/Decl[@kind = 'Parameter']">
<xsl:value-of select="@name"/>
<xsl:if test="not(position()=last())">
   <xsl:text>, int </xsl:text>
</xsl:if>
</xsl:for-each>
</xsl:if>
<xsl:text>)
</xsl:text>

<xsl:text> : smoc_actor(name, </xsl:text> 
<xsl:if test="$schedule_fsm = 0">
   <xsl:text>start</xsl:text>
</xsl:if>
<xsl:if test="$schedule_fsm = 1">
   <xsl:value-of select="/Actor/Schedule/@initial-state"/>
</xsl:if>
<xsl:text>)</xsl:text>

<xsl:variable name="listdef" select="count(/Actor/Decl[@kind = 'Variable' and (count(./Type[@kind]) = 0) and (./Type/@name = 'List') and (./Expr/@kind = 'List')])"/>

<xsl:if test="$param_num > 0">
<xsl:text>, </xsl:text>
<xsl:for-each select="/Actor/Decl[@kind = 'Parameter']">
<xsl:value-of select="@name"/>
<xsl:text>(</xsl:text>
<xsl:choose>
<xsl:when test="count(./Expr) = 1">
<xsl:apply-templates mode="function" select="./Expr"/>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="@name"/>
</xsl:otherwise>
</xsl:choose>
<xsl:text>)</xsl:text>
<xsl:if test="not(position() = last())">
   <xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
</xsl:if>


<xsl:if test="$variable_num - $listdef > 0">
<xsl:text>, </xsl:text>
<xsl:for-each select="/Actor/Decl[@kind = 'Variable' and (count(./Type[@kind]) = 0) and not((./Type/@name = 'List') and (./Expr/@kind = 'List'))]">
<xsl:value-of select="@name"/>
<xsl:text>(</xsl:text>
<xsl:choose>
<xsl:when test="count(./Expr) = 1">
<xsl:apply-templates mode="function" select="./Expr"/>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="@name"/>
</xsl:otherwise>
</xsl:choose>
<xsl:text>)</xsl:text>
<xsl:if test="not(position() = last())">
   <xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>  
</xsl:if>


<xsl:text> {</xsl:text>

<xsl:choose>
<xsl:when test="$schedule_fsm = 0">
   <xsl:text>
 start = 
   </xsl:text>
   <xsl:for-each select="/Actor/Action">
   <xsl:text>(</xsl:text>
   
   <xsl:if test="count(./Input) > 0 or count(./Guards) > 0">
   <xsl:for-each select="./Input">
   <xsl:value-of select="@port"/>
   <xsl:variable name="num_tokens" select="count(./Decl)"/>
   <xsl:text>.getAvailableTokens() &gt;= </xsl:text>
   <xsl:if test="count(./Repeat) = 1">
      <xsl:if test="count(./Repeat/Expr[@kind = 'Literal']) = 1">
      <xsl:value-of select="string($num_tokens * ./Repeat/Expr/@value)"/>
      </xsl:if>
      <xsl:if test="count(./Repeat/Expr[@kind = 'Var']) = 1">
      <xsl:text>(var(</xsl:text>
      <xsl:value-of select="./Repeat/Expr/@name"/>
      <xsl:text>) * </xsl:text>
      <xsl:value-of select="string($num_tokens)"/>
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:if test="count(./Repeat/Expr[@kind = 'BinOpSeq']) = 1">
      <xsl:text>(</xsl:text>
      <xsl:if test="count(./Repeat/Expr/Expr[1][@kind = 'Var']) = 1">
      <xsl:text>var(</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="function" select="./Repeat/Expr/Expr[1]"/>
      <xsl:if test="count(./Repeat/Expr/Expr[1][@kind = 'Var']) = 1">
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="normal" select="./Repeat/Expr/Op"/>
      <xsl:if test="count(./Repeat/Expr/Expr[2][@kind = 'Var']) = 1">
      <xsl:text>var(</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="function" select="./Repeat/Expr/Expr[2]"/>
      <xsl:if test="count(./Repeat/Expr/Expr[2][@kind = 'Var']) = 1">
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:text>)</xsl:text>
      </xsl:if>
   </xsl:if>
   <xsl:if test="count(./Repeat) = 0">
      <xsl:value-of select="string($num_tokens)"/>
   </xsl:if>
   
   <xsl:if test="not(position()=last())">
   <xsl:text> &amp;&amp; 
   </xsl:text>
   </xsl:if>
   </xsl:for-each>
   
   <xsl:if test="count(./Guards) = 1">
   <xsl:if test="count(./Input) > 0">
   <xsl:text> &amp;&amp; 
   </xsl:text>
   </xsl:if> 
   <xsl:text>guard(&amp;m_</xsl:text>
   <xsl:value-of select="/Actor/@name"/>
   <xsl:text>::guard_</xsl:text>
   <xsl:apply-templates select="./QID"/>
   <xsl:text>)</xsl:text>
   </xsl:if>
   </xsl:if>
   
   <xsl:if test="(count(./Guards) = 0) and (count(./Input) = 0)">
   <xsl:text>guard(&amp;m_</xsl:text>
   <xsl:value-of select="/Actor/@name"/>
   <xsl:text>::guard_dummy</xsl:text>
   <xsl:text>)</xsl:text>
   </xsl:if>
   
   <xsl:text>) &gt;&gt; 
   </xsl:text>
   
   <xsl:if test="count(./Output) > 0">
   <xsl:text>(</xsl:text>
   <xsl:for-each select="./Output">
   <xsl:value-of select="@port"/>
   <xsl:text>.getAvailableSpace() &gt;= </xsl:text>
   <xsl:if test="count(./Expr) = 0">
   <xsl:text>1</xsl:text>
   </xsl:if>
   <xsl:if test="count(./Expr) > 0">
   <xsl:value-of select="count(./Expr)"/>
   </xsl:if> 
   <xsl:if test="not(position()=last())">
          <xsl:text> &amp;&amp;  
   </xsl:text>
   </xsl:if>
   </xsl:for-each>
   <xsl:text>) </xsl:text>
   </xsl:if> 
   
   <xsl:text> &gt;&gt;
   </xsl:text>
   <xsl:text>call(&amp;m_</xsl:text>
   <xsl:value-of select="/Actor/@name"/>
   <xsl:text>::</xsl:text>
   <xsl:if test="$action_num = 1">
   <xsl:text>action</xsl:text>
   </xsl:if>
   <xsl:if test="$action_num > 1">
   <xsl:apply-templates select="./QID"/>
   </xsl:if>
   <xsl:text>) &gt;&gt; start</xsl:text>
   <xsl:if test="not(position()=last())">
   <xsl:text> |  
   </xsl:text>
   </xsl:if>
   <xsl:if test="(position()=last())">
          <xsl:text>; </xsl:text>
   </xsl:if>
   </xsl:for-each>
</xsl:when>
<xsl:when test="$schedule_fsm = 1">
<xsl:for-each select="/Actor/Schedule/Transition">

<xsl:variable name="source_state" select="./@from"/>
<xsl:variable name="target_state" select="./@to"/>
<xsl:variable name="action_name" select="./ActionTags/QID/@name"/>

<xsl:for-each select="/Actor/Action/QID">
<xsl:if test="starts-with(@name,$action_name)">
<xsl:text>
</xsl:text>
<xsl:value-of select="$source_state"/>
<xsl:text>.addTransition((</xsl:text>
   <xsl:if test="count(../Input) > 0 or count(../Guards) > 0">
   <xsl:for-each select="../Input">
   <xsl:value-of select="@port"/>
   <xsl:variable name="num_tokens" select="count(./Decl)"/>
   <xsl:text>.getAvailableTokens() &gt;= </xsl:text>
   <xsl:if test="count(./Repeat) = 1">
      <xsl:if test="count(./Repeat/Expr[@kind = 'Literal']) = 1">
      <xsl:value-of select="string($num_tokens * ./Repeat/Expr/@value)"/>
      </xsl:if>
      <xsl:if test="count(./Repeat/Expr[@kind = 'Var']) = 1">
      <xsl:text>(var(</xsl:text>
      <xsl:value-of select="./Repeat/Expr/@name"/>
      <xsl:text>) * </xsl:text>
      <xsl:value-of select="string($num_tokens)"/>
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:if test="count(./Repeat/Expr[@kind = 'BinOpSeq']) = 1">
      <xsl:text>(</xsl:text>
      <xsl:if test="count(./Repeat/Expr/Expr[1][@kind = 'Var']) = 1">
      <xsl:text>var(</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="function" select="./Repeat/Expr/Expr[1]"/>
      <xsl:if test="count(./Repeat/Expr/Expr[1][@kind = 'Var']) = 1">
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="normal" select="./Repeat/Expr/Op"/>
      <xsl:if test="count(./Repeat/Expr/Expr[2][@kind = 'Var']) = 1">
      <xsl:text>var(</xsl:text>
      </xsl:if>
      <xsl:apply-templates mode="function" select="./Repeat/Expr/Expr[2]"/>
      <xsl:if test="count(./Repeat/Expr/Expr[2][@kind = 'Var']) = 1">
      <xsl:text>)</xsl:text>
      </xsl:if>
      <xsl:text>)</xsl:text>
      </xsl:if>
   </xsl:if>
   <xsl:if test="count(./Repeat) = 0">
      <xsl:value-of select="string($num_tokens)"/>
   </xsl:if>
   <xsl:if test="not(position()=last())">
   <xsl:text> &amp;&amp; 
   </xsl:text>
   </xsl:if>
   </xsl:for-each>
   
   <xsl:if test="count(../Guards) = 1">
   <xsl:if test="count(../Input) > 0">
   <xsl:text> &amp;&amp; 
   </xsl:text>
   </xsl:if> 
      <xsl:text>guard(&amp;m_</xsl:text>
      <xsl:value-of select="../../@name"/>
      <xsl:text>::guard_</xsl:text>
      <xsl:apply-templates select="."/>
      <xsl:text>)</xsl:text>
   </xsl:if>
   </xsl:if>
   
   <xsl:if test="(count(../Guards) = 0) and (count(../Input) = 0)">
   <xsl:text>Expr::literal(true)</xsl:text>
   </xsl:if>  
   
   <xsl:text>) &gt;&gt; 
   </xsl:text>
    
   <xsl:if test="count(../Output) > 0">
   <xsl:text>(</xsl:text>
   <xsl:for-each select="../Output">
   <xsl:value-of select="@port"/>
   <xsl:text>.getAvailableSpace() &gt;= </xsl:text> 
   <xsl:if test="count(./Expr) = 0">
   <xsl:text>1</xsl:text>
   </xsl:if>
   <xsl:if test="count(./Expr) > 0">
   <xsl:value-of select="count(./Expr)"/>
   </xsl:if> 
   <xsl:if test="not(position()=last())">
          <xsl:text> &amp;&amp;  
   </xsl:text>
   </xsl:if>
   </xsl:for-each>   
   <xsl:text>) &gt;&gt;
   </xsl:text>
   </xsl:if>
     
   <xsl:text>call(&amp;m_</xsl:text>
   <xsl:value-of select="../../@name"/>
   <xsl:text>::</xsl:text>
   <xsl:apply-templates select="../QID"/>
   <xsl:text>) &gt;&gt; </xsl:text>
   <xsl:value-of select="$target_state"/>
   <xsl:text>); 
   </xsl:text>
   </xsl:if>
   </xsl:for-each>
  </xsl:for-each>
</xsl:when>
</xsl:choose>

<xsl:text> 
 }
};

</xsl:text>
<xsl:call-template name="Epilogue"/>
</xsl:template>



<!--
	Actor Parameter declaration, Variable declaration and Function generator
-->
<xsl:template match="/Actor/Decl">
   <xsl:if test="./@kind = 'Parameter'">
   <xsl:text>const </xsl:text>
   <xsl:value-of select="./Type/@name"/>
   <xsl:text> </xsl:text>
   <xsl:value-of select="@name"/>
   <xsl:text>;
   </xsl:text>  
   </xsl:if>

   <xsl:if test="./@kind = 'Variable'">
   <xsl:if test="not(./Type/@kind = 'Function') and not(./Type/@kind = 'Procedure')">
    <xsl:choose>
   	<xsl:when test="(./Type/@name = 'List') and (count(./Type/descendant::*[@name = 'List']) = 0)">
   	<xsl:choose>
   	<xsl:when test="(./Expr/@kind = 'List') and not(./Expr/Expr/@kind = 'List')">
    <xsl:text>static const int </xsl:text>
    <xsl:value-of select="./@name"/>
    <xsl:text>[];
    </xsl:text>
   	</xsl:when>
   	<xsl:otherwise>
    <xsl:text>cal_list&lt;</xsl:text>
    <xsl:value-of select="./Type/Entry/Type/@name"/>
    <xsl:text>&gt;::t</xsl:text>
    <xsl:text> </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>; 
    </xsl:text>
    </xsl:otherwise>
    </xsl:choose>
    </xsl:when>
    
    
    <xsl:when test="(./Type/@name = 'List') and (count(./Type/descendant::*[@name = 'List']) = 1)">
    <xsl:if test="not(./Expr/@kind = 'List')">
    <xsl:text> cal_list&lt;cal_list&lt;</xsl:text>
    <xsl:value-of select="./Type/Entry/Type/Entry/Type/@name"/>
    <xsl:text>&gt;::t&gt;::t</xsl:text>
    <xsl:text> </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>; 
    </xsl:text>
    </xsl:if> 
    <xsl:if test="(./Expr/@kind = 'List') and (./Expr/Expr/@kind = 'List')">
    <xsl:text>static const int </xsl:text>
    <xsl:value-of select="./@name"/>
    <xsl:text>[</xsl:text>
    <xsl:value-of select="count(./Expr/*)"/>
    <xsl:text>]</xsl:text>
    <xsl:text>[</xsl:text>
    <xsl:value-of select="count(./Expr/Expr[1]/*)"/>
    <xsl:text>];
    </xsl:text>
    </xsl:if>
    </xsl:when>
    
    <xsl:otherwise>
    <xsl:value-of select="./Type/@name"/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>; 
    </xsl:text>
    </xsl:otherwise>
    </xsl:choose>
   
   </xsl:if>
   
   <xsl:if test="./Type/@kind = 'Function'">
   <xsl:choose>
   <xsl:when test="./Type/@name = 'List'">
   <xsl:text>
   cal_list&lt;int&gt;::t </xsl:text>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text>
   int </xsl:text>
   </xsl:otherwise>
   </xsl:choose>
   <xsl:value-of select="@name"/>
   <xsl:text>(</xsl:text>
   <xsl:for-each select="./Expr/Decl[@kind = 'Parameter']">
      <xsl:apply-templates select="."/>
      <xsl:if test="not(position()=last())">
      <xsl:text>, </xsl:text>
      </xsl:if> 
   </xsl:for-each>
   <xsl:text>) const {
   </xsl:text>
   
   
   <xsl:if test="(./Type/@name = 'List') and (count(./Expr/Expr/descendant::*[@kind = 'Generator']) >= 1)">
   <xsl:text>   cal_list&lt;int&gt;::t ret_list;
   </xsl:text>
   </xsl:if>
   
   <xsl:for-each select="./Expr/Decl[@kind = 'Variable']">
   <xsl:choose>
   <xsl:when test="count(./Type[@name = 'List']) = 1">
   <xsl:text>   cal_list&lt;</xsl:text>
   <xsl:value-of select="./Type/Entry/Type/@name"/>
   <xsl:text>&gt;::t </xsl:text>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text>   int </xsl:text>
   </xsl:otherwise>
   </xsl:choose>
   <xsl:value-of select="@name"/>
   
   <xsl:if test="count(./Expr) = 1">
   <xsl:choose>
   <xsl:when test="(./Type/@name = 'List') and (count(./Expr/descendant::*[@kind = 'Generator']) >= 1)">
   <xsl:text>;
   </xsl:text>
   <xsl:apply-templates mode="function" select="./Expr">
      <xsl:with-param name="var_name">
      <xsl:value-of select="@name"/>
      </xsl:with-param>
   </xsl:apply-templates>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text> = </xsl:text>
   <xsl:apply-templates mode="function" select="./Expr"/> 
   </xsl:otherwise>
   </xsl:choose>
   </xsl:if>
   
   <xsl:text>;
   </xsl:text>
   </xsl:for-each>
   
   <xsl:choose>
   <xsl:when test="(./Type/@name = 'List') and (count(./Expr/Expr/descendant::*[@kind = 'Generator']) >= 1)">
     <xsl:apply-templates mode="normal" select="./Expr/Expr">
        <xsl:with-param name="var_name">ret_list</xsl:with-param>
     </xsl:apply-templates>
   <xsl:text>
   return( ret_list );</xsl:text>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text>return( </xsl:text>
   <xsl:apply-templates mode='function' select="./Expr/Expr"/>
   <xsl:text> );</xsl:text>
   </xsl:otherwise>
   </xsl:choose>
   <xsl:text>
   }
   
   </xsl:text>
   
   </xsl:if>
   </xsl:if>
   
   

   <xsl:if test="./Type/@kind = 'Procedure'">
   <xsl:text>
   void </xsl:text>
   <xsl:value-of select="@name"/>
   <xsl:text>(</xsl:text>
   <xsl:for-each select="./Expr/Decl[@kind = 'Parameter']">
      <xsl:apply-templates select="."/>
      <xsl:if test="not(position()=last())">
      <xsl:text>, </xsl:text>
      </xsl:if> 
   </xsl:for-each>
   <xsl:text>) {
</xsl:text>
<xsl:for-each select="./Expr/Stmt">
<xsl:apply-templates select="."/>
</xsl:for-each>
<xsl:text>   }
 
   </xsl:text>
   </xsl:if>
    


</xsl:template>


<!--
	Actor Port Declarations generator
-->
<xsl:template match="/Actor/Port">
<xsl:text>   smoc_port_</xsl:text>
<xsl:choose>
   <xsl:when test="@kind='Input'"><xsl:text>in</xsl:text></xsl:when>
   <xsl:when test="@kind='Output'"><xsl:text>out</xsl:text></xsl:when>
   <xsl:otherwise><xsl:text>ERROR in Actor port kind!</xsl:text></xsl:otherwise>
</xsl:choose>
<xsl:text>&lt;</xsl:text>

<xsl:if test="./Type/@name = 'int'">
<xsl:text>int</xsl:text>
<xsl:text>&gt; </xsl:text> 
</xsl:if>

<xsl:if test="./Type/@name = 'List'">
<xsl:text>cal_list&lt;</xsl:text>
<xsl:value-of select="./Type/Entry/Type/@name"/>
<xsl:text>&gt;::t&gt; </xsl:text>
</xsl:if>

<xsl:value-of select="@name"/>
<xsl:text>;
</xsl:text>

</xsl:template>



<!--
	Actor Action Generator
	(uses names of cal as action names, except if actor has only one action: action)
-->
<xsl:template match="/Actor/Action">
<xsl:text>void m_</xsl:text>
<xsl:value-of select="/Actor/@name"/>
<xsl:text>::</xsl:text>
<xsl:if test="count(/Actor/Action) = 1">
  <xsl:text>action</xsl:text>
</xsl:if>
<xsl:if test="count(/Actor/Action) > 1">
  <xsl:apply-templates select="./QID"/>
</xsl:if>
<xsl:text>(void) {
</xsl:text>

<!--
 Tell us how many local variable declarations the action has
-->
<xsl:variable name="var_num" select="count(./Decl[@kind='Variable'])"/>
<xsl:text>// The action has </xsl:text>
<xsl:value-of select="$var_num"/>
<xsl:text> local variable declarations.
</xsl:text>

<!--
 Generate all Input declarations of the action
-->
<xsl:for-each select="./Input">
<xsl:for-each select="./Decl">
<xsl:variable name="input_name" select="../@port"/>

<xsl:if test="count(../Repeat) = 1">
<xsl:variable name="help_name" select="@name"/>
<xsl:variable name="termin_expr">
  <xsl:apply-templates mode="function" select="../Repeat/Expr"/>
</xsl:variable>
<xsl:for-each select="/Actor/Port[@kind='Input' and starts-with(@name,$input_name)]">
<xsl:text>   cal_list&lt;int&gt;::t </xsl:text>
<xsl:value-of select="$help_name"/>
<xsl:text>;</xsl:text>
<xsl:text> for (unsigned int i=0; i&lt;</xsl:text>
<xsl:value-of select="$termin_expr"/>
<xsl:text>; i++) </xsl:text>
<xsl:value-of select="$help_name"/>
<xsl:text>.push_back(</xsl:text>
<xsl:value-of select="$input_name"/>
<xsl:text>[i]);
</xsl:text>
</xsl:for-each>
</xsl:if>

<xsl:if test="count(../Repeat) = 0">
<xsl:for-each select="/Actor/Port[@kind='Input' and starts-with(@name,$input_name)]">
<xsl:choose>
<xsl:when test="count(./Type[@name = 'List']) = 1">
<xsl:text>   const cal_list&lt;</xsl:text>
<xsl:value-of select="./Type/Entry/Type/@name"/>
<xsl:text>&gt;::t &amp;</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:text>   int </xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
<xsl:value-of select="./@name"/>
<xsl:text> = </xsl:text>
<xsl:value-of select="../@port"/>
<xsl:text>[</xsl:text>
<xsl:value-of select="position() -1"/>
<xsl:text>];
</xsl:text>
</xsl:if>

</xsl:for-each>
</xsl:for-each>

<!--
 Generate all Variable Declarations inside action:
 consider case a) list variable and b) integer variable
-->
<xsl:if test="$var_num > 0">
<xsl:for-each select="./Decl[@kind='Variable']">
<xsl:choose>
<!--
 a) generate list variable declarations inside action
-->
<xsl:when test="count(./Type[@name = 'List']) = 1">
<xsl:text>   cal_list&lt;</xsl:text>
<xsl:value-of select="./Type/Entry/Type/@name"/>
<xsl:text>&gt;::t </xsl:text>
<xsl:value-of select="@name"/>
<xsl:if test="count(./Expr) = 1">
   <xsl:choose>
   <xsl:when test="(./Expr/@kind = 'List')">
   <xsl:text>;</xsl:text>
   <xsl:if test="count(./Expr/Generator) = 1">
     <xsl:apply-templates select="./Expr/Generator">
         <xsl:with-param name="var_name">
         <xsl:value-of select="@name"/>
         </xsl:with-param>
     </xsl:apply-templates>
   </xsl:if>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text> = </xsl:text>
   <xsl:apply-templates mode="function" select="./Expr"/>
   <xsl:text>;
</xsl:text> 
   </xsl:otherwise>
   </xsl:choose>
</xsl:if>
</xsl:when>
<!--
 b) generate non-list variable declarations inside action
-->
<xsl:otherwise>
<xsl:text>   int </xsl:text>
<xsl:value-of select="@name"/>
<xsl:if test="count(./Expr) = 1">
<xsl:text> = </xsl:text>
<xsl:apply-templates mode="function" select="./Expr"/> 
</xsl:if>
<xsl:text>;
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
</xsl:if>


<!--
 generate all action statements
-->
<xsl:for-each select="./Stmt">
<xsl:apply-templates select="."/>
</xsl:for-each>


<!--
 generate all action outputs
-->
<xsl:for-each select="./Output">
<xsl:variable name="Expr_count" select="count(./Expr)"/>
<xsl:for-each select="./Expr">
<xsl:text>   </xsl:text>
<xsl:value-of select="../@port"/>
<xsl:text>[</xsl:text>
<xsl:number value="position() - 1"/>

<xsl:variable name="output_name" select="../@port"/>

<xsl:for-each select="/Actor/Port[@kind='Output' and starts-with(@name,$output_name)]">
<xsl:choose>
<xsl:when test="count(./Type[@name = 'List']) = 1">
<xsl:text>] = </xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:text>] = </xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>

<xsl:apply-templates mode="function" select="."/>
<xsl:text>;
</xsl:text>
</xsl:for-each>
</xsl:for-each>

<xsl:text>}
</xsl:text>
</xsl:template>


<!--
	Function generator for Actor Guards
-->
<xsl:template match="/Actor/Action/Guards">
<xsl:text>bool m_</xsl:text>
<xsl:value-of select="/Actor/@name"/>
<xsl:text>::guard_</xsl:text>
<xsl:apply-templates select="../QID"/>
<xsl:text>(void)  const {
</xsl:text>

<xsl:variable name="var_num" select="count(../Decl[@kind='Variable'])"/>

<xsl:for-each select="../Input">
<xsl:for-each select="./Decl">
<xsl:variable name="input_name" select="../@port"/>

<xsl:if test="count(../Repeat) = 1">
<xsl:variable name="help_name" select="@name"/>
<xsl:variable name="termin_expr">
  <xsl:apply-templates mode="function" select="../Repeat/Expr"/>
</xsl:variable>
<xsl:for-each select="/Actor/Port[@kind='Input' and starts-with(@name,$input_name)]">
<xsl:text>   cal_list&lt;int&gt;::t </xsl:text>
<xsl:value-of select="$help_name"/>
<xsl:text>;</xsl:text>
<xsl:text> for (unsigned int i=0; i&lt;</xsl:text>
<xsl:value-of select="$termin_expr"/>
<xsl:text>; i++) </xsl:text>
<xsl:value-of select="$help_name"/>
<xsl:text>.push_back(</xsl:text>
<xsl:value-of select="$input_name"/>
<xsl:text>[i]);
</xsl:text>
</xsl:for-each>
</xsl:if>

<xsl:if test="count(../Repeat) = 0">
<xsl:for-each select="/Actor/Port[@kind='Input' and starts-with(@name,$input_name)]">
<xsl:choose>
<xsl:when test="count(./Type[@name = 'List']) = 1">
<xsl:text>   const cal_list&lt;</xsl:text>
<xsl:value-of select="./Type/Entry/Type/@name"/>
<xsl:text>&gt;::t &amp;</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:text>   const int </xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
<xsl:value-of select="./@name"/>
<xsl:text> = </xsl:text>
<xsl:value-of select="../@port"/>
<xsl:text>[</xsl:text>
<xsl:value-of select="position() -1"/>
<xsl:text>];
</xsl:text>
</xsl:if>

</xsl:for-each>
</xsl:for-each>


<!--
 Generate all Variable Declarations inside corresponding action as these might be used also in guard
 consider case a) list variable and b) integer variable
-->
<xsl:if test="$var_num > 0">
<xsl:for-each select="../Decl[@kind='Variable']">
<xsl:choose>
<!--
 a) generate list variable declarations inside corresponding action
-->
<xsl:when test="count(./Type[@name = 'List']) = 1">
<xsl:text>   cal_list&lt;</xsl:text>
<xsl:value-of select="./Type/Entry/Type/@name"/>
<xsl:text>&gt;::t </xsl:text>
<xsl:value-of select="@name"/>
<xsl:if test="count(./Expr) = 1">
   <xsl:choose>
   <xsl:when test="(./Expr/@kind = 'List')">
   <xsl:text>;</xsl:text>
   <xsl:if test="count(./Expr/Generator) = 1">
     <xsl:apply-templates select="./Expr/Generator">
         <xsl:with-param name="var_name">
         <xsl:value-of select="@name"/>
         </xsl:with-param>
     </xsl:apply-templates>
   </xsl:if>
   </xsl:when>
   <xsl:otherwise>
   <xsl:text> = </xsl:text>
   <xsl:apply-templates mode="function" select="./Expr"/>
   <xsl:text>;
</xsl:text> 
   </xsl:otherwise>
   </xsl:choose>
</xsl:if>
</xsl:when>
<!--
 b) generate non-list variable declarations inside corresponding action
-->
<xsl:otherwise>
<xsl:text>   int </xsl:text>
<xsl:value-of select="@name"/>
<xsl:if test="count(./Expr) = 1">
<xsl:text> = </xsl:text>
<xsl:apply-templates mode="function" select="./Expr"/> 
</xsl:if>
<xsl:text>;
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
</xsl:if>



<xsl:text>   return( </xsl:text>
<xsl:for-each select="./Expr">
<xsl:apply-templates mode="function" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text> &amp;&amp; </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text> );
</xsl:text>
<xsl:text>}
</xsl:text>
</xsl:template>


<!--
    Expression generator (calls itself recursively) mode=function
-->
<xsl:template match="Expr" mode="function">
<xsl:param name="var_name">ret_list</xsl:param>

<xsl:if test="./@kind = 'Literal'">
<xsl:value-of select="./@value"/>
</xsl:if>

<xsl:if test="./@kind = 'Var'">
<xsl:if test="(./@name = 'bitand') or (./@name = 'bitor')">
<xsl:text>cal_</xsl:text>
</xsl:if>
<xsl:value-of select="./@name"/>
</xsl:if>

<xsl:if test="./@kind = 'UnaryOp'">
<xsl:text>(</xsl:text>
<xsl:choose>
<xsl:when test="./Op/@name = 'not'">
<xsl:text>! </xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="./Op/@name "/>
</xsl:otherwise>
</xsl:choose>
<xsl:apply-templates mode="function" select="./Expr"/>
<xsl:text>)</xsl:text>
</xsl:if> 


<xsl:if test="./@kind = 'BinOpSeq'">
<xsl:text>(</xsl:text>
<xsl:variable name="op_count" select="count(./Op)"/>

<xsl:apply-templates mode="function" select="./Expr[1]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="function" select="./Op[1]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="function" select="./Expr[2]"/>

<xsl:if test="$op_count > 1">
<xsl:text></xsl:text>
<xsl:apply-templates mode="function" select="./Op[2]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="function" select="./Op[2]/following-sibling::*"/>
<xsl:text></xsl:text>
<xsl:text>)</xsl:text>
</xsl:if> 

<xsl:if test="$op_count = 1">
<xsl:text>)</xsl:text>
</xsl:if>
</xsl:if>


<xsl:if test="./@kind = 'If'">
<xsl:text></xsl:text>
<xsl:apply-templates mode="function" select="./Expr[1]"/>
<xsl:text> ? </xsl:text>
<xsl:apply-templates mode="function" select="./Expr[2]"/>
<xsl:text> :</xsl:text>
<xsl:if test="count(./Expr) > 2">
<xsl:text> (</xsl:text>
<xsl:apply-templates mode="function" select="./Expr[3]"/>
<xsl:text>)</xsl:text>
</xsl:if>
</xsl:if>


<xsl:if test="./@kind = 'Application'">
<xsl:choose>
<xsl:when test="./Expr/@name = 'get'">
<xsl:apply-templates mode="function" select="./Expr/Expr"/>
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="function" select="./Args/Expr"/>
<xsl:text>]</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:apply-templates mode="function" select="./Expr"/>
<xsl:text>(</xsl:text>
<xsl:for-each select="./Args/Expr">
<xsl:apply-templates mode="function" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>)</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:if>

<xsl:if test="./@kind = 'List'">
<xsl:apply-templates select="./Generator">
<xsl:with-param name="var_name">
   <xsl:value-of select="$var_name"/>
   </xsl:with-param>
</xsl:apply-templates>
</xsl:if>

<xsl:if test="./@kind = 'Indexer'">
<xsl:apply-templates mode="function" select="./Expr"/>
<xsl:for-each select="./Args/Expr">
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="function" select="."/>
<xsl:text>]</xsl:text>
</xsl:for-each>
</xsl:if>

<xsl:if test="./@kind = 'Entry'">
<xsl:text>EXPR: Error: Entries appear only in Stmt!</xsl:text>
</xsl:if>

<xsl:if test="./@kind = 'Proc'">
<xsl:text>EXPR: Error: Proc appears only in Stmt!</xsl:text>
</xsl:if>

</xsl:template>


<!--
    Expression generator (calls itself recursively) mode=normal
-->
<xsl:template match="Expr" mode="normal">
<xsl:param name="var_name">ret_list</xsl:param>

<xsl:if test="./@kind = 'Literal'">
<xsl:value-of select="./@value"/>
</xsl:if>

<xsl:if test="./@kind = 'Var'">
<xsl:if test="(./@name = 'bitand') or (./@name = 'bitor')">
<xsl:text>cal_</xsl:text>
</xsl:if>
<xsl:value-of select="./@name"/>
</xsl:if>

<xsl:if test="./@kind = 'UnaryOp'">
<xsl:text>(</xsl:text>
<xsl:choose>
<xsl:when test="./Op/@name = 'not'">
<xsl:text>! </xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="./Op/@name "/>
</xsl:otherwise>
</xsl:choose>
<xsl:apply-templates mode="normal" select="./Expr"/>
<xsl:text>)</xsl:text>
</xsl:if> 


<xsl:if test="./@kind = 'BinOpSeq'">
<xsl:text>(</xsl:text>
<xsl:variable name="op_count" select="count(./Op)"/>

<xsl:apply-templates mode="normal" select="./Expr[1]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="normal" select="./Op[1]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="normal" select="./Expr[2]"/>

<xsl:if test="$op_count > 1">
<xsl:text></xsl:text>
<xsl:apply-templates mode="normal" select="./Op[2]"/>
<xsl:text></xsl:text>
<xsl:apply-templates mode="normal" select="./Op[2]/following-sibling::*"/>
<xsl:text></xsl:text>
<xsl:text>)</xsl:text>
</xsl:if> 

<xsl:if test="$op_count = 1">
<xsl:text>)</xsl:text>
</xsl:if>
</xsl:if>


<xsl:if test="./@kind = 'If'">
<xsl:text>if </xsl:text>
<xsl:apply-templates mode="normal" select="./Expr[1]"/>
<xsl:text> 
     { </xsl:text>
<xsl:apply-templates mode="normal" select="./Expr[2]"/>
<xsl:text>
     }</xsl:text>
<xsl:if test="count(./Expr) > 2">
<xsl:text> 
     else { </xsl:text>
<xsl:apply-templates mode="normal" select="./Expr[3]"/>
<xsl:text>
     }</xsl:text>
</xsl:if>
</xsl:if>

<xsl:if test="./@kind = 'Application'">
<xsl:choose>
<xsl:when test="./Expr/@name = 'get'">
<xsl:apply-templates mode="normal" select="./Expr/Expr"/>
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="normal" select="./Args/Expr"/>
<xsl:text>]</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:apply-templates mode="normal" select="./Expr"/>
<xsl:text>(</xsl:text>
<xsl:for-each select="./Args/Expr">
<xsl:apply-templates mode="normal" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>)</xsl:text>
</xsl:otherwise>
</xsl:choose>

</xsl:if>


<xsl:if test="./@kind = 'List'">
<xsl:apply-templates select="./Generator">
   <xsl:with-param name="var_name">
   <xsl:value-of select="$var_name"/>
   </xsl:with-param>
</xsl:apply-templates>
</xsl:if>

<xsl:if test="./@kind = 'Indexer'">
<xsl:apply-templates mode="normal" select="./Expr"/>
<xsl:for-each select="./Args/Expr">
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="normal" select="."/>
<xsl:text>]</xsl:text>
</xsl:for-each>
</xsl:if>

<xsl:if test="./@kind = 'Entry'">
<xsl:text>EXPR:Error: Entries appear only in Stmt!</xsl:text>
</xsl:if>

<xsl:if test="./@kind = 'Proc'">
<xsl:text>EXPR: Error: Proc appear only in Stmt!</xsl:text>
</xsl:if>

</xsl:template>



<!--
    Statement generator (calls itself recursively)
-->
<xsl:template match="Stmt">

<xsl:if test="./@kind = 'Assign'">
<xsl:choose>
<xsl:when test="./Expr/@kind = 'List'">
<xsl:for-each select="./Expr/Expr">
<xsl:text>   </xsl:text>
<xsl:value-of select="../../@name"/>
<xsl:text>[</xsl:text>
<xsl:number value="position() - 1"/>
<xsl:text>] = </xsl:text>
<xsl:value-of select="./@value"/>
<xsl:text>;
</xsl:text>
<xsl:if test="not(position()=last())">
<xsl:text></xsl:text>
</xsl:if> 
</xsl:for-each>
</xsl:when>
<xsl:otherwise>
<xsl:text>   </xsl:text>
<xsl:value-of select="./@name"/>
<xsl:if test="count(./Args) = 1">
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="function" select="./Args/Expr"/>
<xsl:text>]</xsl:text>
</xsl:if>
<xsl:text> = </xsl:text>
<xsl:apply-templates mode="function" select="./Expr"/>
<xsl:text>; 
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:if>

<xsl:if test="./@kind = 'Block'">
<xsl:text>{ 
</xsl:text>
<xsl:for-each select="./Stmt">
<xsl:text>   </xsl:text>
<xsl:apply-templates select="."/>
</xsl:for-each>
<xsl:text>   }
</xsl:text>
</xsl:if>

<xsl:if test="./@kind = 'If'">
<xsl:text>   if </xsl:text>
<xsl:apply-templates mode="normal" select="./Expr"/>
<xsl:text> </xsl:text>
<xsl:apply-templates  select="./Stmt[1]"/>
<xsl:if test="count(./Stmt) = 2">
<xsl:text>   else </xsl:text>
<xsl:apply-templates select="./Stmt[2]"/>
</xsl:if>
</xsl:if>


<xsl:if test="./@kind = 'Call'">
<xsl:text>   </xsl:text>
<xsl:choose>
<xsl:when test="./Expr/@name = 'get'">
<xsl:apply-templates mode="normal" select="./Expr/Expr"/>
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="normal" select="./Args/Expr"/>
<xsl:text>]</xsl:text>
</xsl:when>

<xsl:when test="./Expr/@name = 'set'">
<xsl:choose>
<xsl:when test="(./Args/Expr[1]/@kind = 'List') and not(./Args/Expr[2]/@kind = 'List')">
<xsl:apply-templates mode="normal" select="./Args/Expr[1]">
   <xsl:with-param name="var_name">
   <xsl:apply-templates mode="normal" select="./Expr/Expr[1]"/>
   <xsl:text>[</xsl:text>
   <xsl:apply-templates mode="normal" select="./Args/Expr[2]"/>
   <xsl:text>]</xsl:text>
   </xsl:with-param>
</xsl:apply-templates>
</xsl:when>
<xsl:when test="(./Args/Expr[2]/@kind = 'List') and not(./Args/Expr[1]/@kind = 'List')">
<xsl:apply-templates mode="normal" select="./Args/Expr[2]">
   <xsl:with-param name="var_name">
   <xsl:apply-templates mode="normal" select="./Expr/Expr[1]"/>
   <xsl:text>[</xsl:text>
   <xsl:apply-templates mode="normal" select="./Args/Expr[1]"/>
   <xsl:text>]</xsl:text>
   </xsl:with-param>
</xsl:apply-templates>
</xsl:when>
<xsl:when test="(./Args/Expr[1]/@kind = 'List') and (./Args/Expr[2]/@kind = 'List')">
<xsl:text>Stmt: Call: ERR: Multiple array arguments being list constructors not yet supported!
</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:apply-templates mode="function" select="./Expr/Expr"/>
<xsl:text>[</xsl:text>
<xsl:apply-templates mode="function" select="./Args/Expr[1]"/>
<xsl:text>] = </xsl:text>
<xsl:apply-templates mode="function" select="./Args/Expr[2]"/>
<xsl:text>;
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:when>

<xsl:otherwise>
<xsl:if test="./Expr/@name = 'println'">
<xsl:text>//</xsl:text>
</xsl:if>
<xsl:value-of select="./Expr/@name"/>
<xsl:text>(</xsl:text>
<xsl:for-each select="./Args/Expr">
<xsl:apply-templates mode="normal" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>);
</xsl:text>
</xsl:otherwise>
</xsl:choose>
</xsl:if>

</xsl:template>


<!--
    Operator generator
-->
<xsl:template match="Op" mode="function normal">
<xsl:text> </xsl:text>
<xsl:choose>
<xsl:when test="./@name = '='">
<xsl:text>==</xsl:text>
</xsl:when>
<xsl:when test="./@name = 'and'">
<xsl:text>&amp;&amp;</xsl:text>
</xsl:when>
<xsl:when test="./@name = 'or'">
<xsl:text>||</xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="./@name"/>
</xsl:otherwise>
</xsl:choose>
<xsl:text> </xsl:text>
</xsl:template>


<!--
    Parameter declaration generator (used in Function and Procedure)
-->
<xsl:template match="Decl">
<xsl:choose>
<xsl:when test="./Type/@name = 'List'">
<xsl:text>cal_list&lt;int&gt;::t </xsl:text>
</xsl:when>
<xsl:when test="./Type/@name = 'ListofList'">
<xsl:text>cal_list&lt;cal_list&lt;int&gt;::t&gt;::t </xsl:text>
</xsl:when>
<xsl:otherwise>
<xsl:text>int </xsl:text>  
</xsl:otherwise>
</xsl:choose>
<xsl:value-of select="@name"/>
</xsl:template>


<!--
    Generator of for-loop string
-->
<xsl:template match="Generator">
<xsl:param name="var_name">BERNI</xsl:param>
<xsl:choose>
<xsl:when test="count(./Expr) = 1 and (./Expr/@kind = 'Application')">
<xsl:if test="(./Expr/Expr/@kind = 'Var') and (./Expr/Expr/@name = 'Integers')">
<xsl:text>
</xsl:text>
<xsl:for-each select="./Decl">
<xsl:text>     for (unsigned int </xsl:text>
<xsl:value-of select="./@name"/>
<xsl:text> = </xsl:text>
<xsl:apply-templates mode="function" select="../Expr/Args/Expr[1]"/>
<xsl:text>; </xsl:text>
<xsl:value-of select="./@name"/>
<xsl:text> &lt;= </xsl:text>
<xsl:apply-templates mode="function" select="../Expr/Args/Expr[2]"/>
<xsl:text>; ++</xsl:text>
<xsl:value-of select="./@name"/>
<xsl:text>) 
                </xsl:text>
</xsl:for-each>
<xsl:text>     {      
</xsl:text>
<xsl:text>        </xsl:text>
<xsl:value-of select="$var_name"/>
<xsl:text>.push_back(</xsl:text>
<xsl:apply-templates mode="function" select="../Expr"/>
<xsl:text>);   
     }
</xsl:text>
</xsl:if>
</xsl:when>

<xsl:when test="count(./Expr) = 1 and (./Expr/@kind = 'List')">
<xsl:text>const int indices[] = {</xsl:text>
<xsl:for-each select="./Expr/Expr">
<xsl:value-of select="./@value"/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>};
</xsl:text>
<xsl:text>   for (unsigned int iter = 0; </xsl:text>
<xsl:text>iter &lt; sizeof(indices)/sizeof(indices[0]); ++iter)</xsl:text>
<xsl:text>     {         
</xsl:text>
<xsl:text>        </xsl:text>
<xsl:value-of select="$var_name"/>
<xsl:text>[iter] = </xsl:text>
<xsl:apply-templates mode="function" select="../Expr"/>
<xsl:text>;   
     }
</xsl:text>
</xsl:when>

<xsl:otherwise>
<xsl:apply-templates mode="function" select="../Expr"/>
</xsl:otherwise>

</xsl:choose>
</xsl:template>


<!--
	Name generator for actions and guards
-->
<xsl:template match="QID">
<xsl:choose>
<xsl:when test="contains(@name,'.')">
<xsl:value-of select="substring-before(@name,'.')"/>
<xsl:text>_</xsl:text>
 <xsl:choose>
   <xsl:when test="contains(substring-after(@name,'.'),'.')">
   <xsl:value-of select="substring-before(substring-after(@name,'.'),'.')"/>
   <xsl:text>_</xsl:text>
   <xsl:value-of select="substring-after(substring-after(@name,'.'),'.')"/>
   </xsl:when>
   <xsl:otherwise>
   <xsl:value-of select="substring-after(@name,'.')"/>
   </xsl:otherwise>
   </xsl:choose>
</xsl:when>
<xsl:otherwise>
<xsl:value-of select="@name"/>
</xsl:otherwise>
</xsl:choose>
</xsl:template>

<!--
   Epilogue generator: create all initializations of class variables at the end of the program
-->
<xsl:template name="Epilogue">
<xsl:for-each select="/Actor/Decl">
<xsl:if test="(./@kind = 'Variable') and not(./Type/@kind = 'Function') and not(./Type/@kind = 'Procedure')">
<xsl:choose>
<xsl:when test="(./Type/@name = 'List') and (count(./Type/descendant::*[@name = 'List']) = 0)">
<xsl:if test="(./Expr/@kind = 'List') and not(./Expr/Expr/@kind = 'List')">
<xsl:text>const int m_</xsl:text>
<xsl:value-of select="/Actor/@name"/>
<xsl:text>::</xsl:text>
<xsl:value-of select="./@name"/>
<xsl:text>[</xsl:text>
<xsl:text>]</xsl:text>
<xsl:text>= { 
</xsl:text>
<xsl:for-each select="./Expr/Expr">
<xsl:apply-templates mode="function" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>
};

</xsl:text>
</xsl:if>
</xsl:when>
<xsl:when test="(./Type/@name = 'List') and (count(./Type/descendant::*[@name = 'List']) = 1)">
<xsl:if test="(./Expr/@kind = 'List') and (./Expr/Expr/@kind = 'List')">
<xsl:text>const int m_</xsl:text>
<xsl:value-of select="/Actor/@name"/>
<xsl:text>::</xsl:text>
<xsl:value-of select="./@name"/>
<xsl:text>[</xsl:text>
<xsl:value-of select="count(./Expr/*)"/>
<xsl:text>]</xsl:text>
<xsl:text>[</xsl:text>
<xsl:value-of select="count(./Expr/Expr[1]/*)"/>
<xsl:text>] = {
</xsl:text>
<xsl:for-each select="./Expr/Expr">
<xsl:text>{ </xsl:text>
<xsl:for-each select="./Expr">
<xsl:text> </xsl:text>
<xsl:apply-templates mode="function" select="."/>
<xsl:if test="not(position()=last())">
<xsl:text>, </xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text> }</xsl:text>
<xsl:if test="not(position()=last())">
<xsl:text>, 
</xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:text>
};

</xsl:text>
</xsl:if>
</xsl:when>
<xsl:otherwise>
</xsl:otherwise>
</xsl:choose>
</xsl:if>
</xsl:for-each>
</xsl:template>

</xsl:stylesheet>


